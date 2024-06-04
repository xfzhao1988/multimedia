/*!********************************************************************
*
 Audacity: A Digital Audio Editor

 WaveTrackAffordanceControls.cpp

 Vitaly Sverchinsky

 **********************************************************************/

#include "WaveTrackAffordanceControls.h"

#include <wx/dc.h>
#include <wx/frame.h>

#include "AllThemeResources.h"
#include "CommandContext.h"
#include "CommandFlag.h"
#include "CommandFunctors.h"
#include "MenuRegistry.h"
#include "../../../../TrackPanelMouseEvent.h"
#include "../../../../TrackArt.h"
#include "../../../../TrackArtist.h"
#include "../../../../TrackPanelDrawingContext.h"
#include "ViewInfo.h"
#include "WaveTrack.h"
#include "WaveClip.h"
#include "WaveTrackUtilities.h"
#include "UndoManager.h"
#include "ShuttleGui.h"
#include "../../../../ProjectWindows.h"
#include "../../../../commands/AudacityCommand.h"

#include "../../../ui/SelectHandle.h"
#include "../../../ui/TextEditHelper.h"
#include "ClipParameters.h"
#include "WaveChannelView.h"
#include "WaveTrackAffordanceHandle.h"

#include "ProjectHistory.h"
#include "../../../../ProjectSettings.h"
#include "SelectionState.h"
#include "../../../../RefreshCode.h"
#include "Theme.h"
#include "../../../../../images/Cursors.h"
#include "../../../../HitTestResult.h"
#include "../../../../TrackPanel.h"
#include "TrackFocus.h"

#include "../WaveTrackUtils.h"

#include "ClipOverflowButtonHandle.h"
#include "ClipPitchAndSpeedButtonHandle.h"
#include "LowlitClipButton.h"
#include "PitchAndSpeedDialog.h"
#include "WaveClipAdjustBorderHandle.h"
#include "WaveClipUtilities.h"

#include "BasicUI.h"
#include "UserException.h"


class SetWaveClipNameCommand : public AudacityCommand
{
public:
    static const ComponentInterfaceSymbol Symbol;

    ComponentInterfaceSymbol GetSymbol() const override
    {
        return Symbol;
    }
    void PopulateOrExchange(ShuttleGui& S) override
    {
        S.AddSpace(0, 5);

        S.StartMultiColumn(2, wxALIGN_CENTER);
        {
            S.TieTextBox(XXO("Name:"), mName, 60);
        }
        S.EndMultiColumn();
    }
public:
    wxString mName;
};

const ComponentInterfaceSymbol SetWaveClipNameCommand::Symbol
{ XO("Set Wave Clip Name") };

//Handle which is used to send mouse events to TextEditHelper
class WaveClipTitleEditHandle final : public UIHandle
{
    std::shared_ptr<TextEditHelper> mHelper;
    std::shared_ptr<WaveTrack> mpTrack;
public:

    WaveClipTitleEditHandle(const std::shared_ptr<TextEditHelper>& helper,
        const std::shared_ptr<WaveTrack> pTrack)
        : mHelper(helper)
        , mpTrack{ move(pTrack) }
    { }

   ~WaveClipTitleEditHandle()
   {
   }

    std::shared_ptr<const Channel> FindChannel() const override
    {
        return mpTrack;
    }

    Result Click(const TrackPanelMouseEvent& event, AudacityProject* project) override
    {
        if (mHelper->OnClick(event.event, project))
            return RefreshCode::RefreshCell;
        return RefreshCode::RefreshNone;
    }

    Result Drag(const TrackPanelMouseEvent& event, AudacityProject* project) override
    {
        if (mHelper->OnDrag(event.event, project))
            return RefreshCode::RefreshCell;
        return RefreshCode::RefreshNone;
    }

    HitTestPreview Preview(const TrackPanelMouseState& state, AudacityProject* pProject) override
    {
        static auto ibeamCursor =
            ::MakeCursor(wxCURSOR_IBEAM, IBeamCursorXpm, 17, 16);
        return {
           XO("Click and drag to select text"),
           ibeamCursor.get()
        };
    }

    Result Release(const TrackPanelMouseEvent& event, AudacityProject* project, wxWindow*) override
    {
        if (mHelper->OnRelease(event.event, project))
            return RefreshCode::RefreshCell;
        return RefreshCode::RefreshNone;
    }

    Result Cancel(AudacityProject* project) override
    {
        if (mHelper)
        {
            mHelper->Cancel(project);
            mHelper.reset();
        }
        return RefreshCode::RefreshAll;
    }
};

WaveTrackAffordanceControls::WaveTrackAffordanceControls(const std::shared_ptr<Track>& pTrack)
    : CommonTrackCell{ pTrack, 0 }
    , mClipNameFont{ wxFontInfo{} }
{
    if (auto trackList = pTrack->GetOwner())
    {
        mTrackListEventSubscription = trackList->Subscribe(
            *this, &WaveTrackAffordanceControls::OnTrackListEvent);
        if(auto project = trackList->GetOwner())
        {
            auto& viewInfo = ViewInfo::Get(*project);
            mSelectionChangeSubscription =
                viewInfo.selectedRegion.Subscribe(
                    *this,
                    &WaveTrackAffordanceControls::OnSelectionChange);
        }
    }
}

std::vector<UIHandlePtr> WaveTrackAffordanceControls::HitTest(const TrackPanelMouseState& state, const AudacityProject* pProject)
{
    std::vector<UIHandlePtr> results;

    const auto px = state.state.m_x;
    const auto py = state.state.m_y;
    const wxPoint mousePoint { px, py };

    const auto rect = state.rect;

    auto track = std::static_pointer_cast<WaveTrack>(FindTrack());
    // Assume only leader channels have affordance areas
    assert(track->IsLeader());

    {
        auto handle = WaveClipAdjustBorderHandle::HitAnywhere(
            mClipBorderAdjustHandle,
            track,
            pProject,
            state);

        if (handle)
            results.push_back(handle);
    }

    if (mTextEditHelper && mTextEditHelper->GetBBox().Contains(px, py))
    {
        results.push_back(
            AssignUIHandlePtr(
                mTitleEditHandle,
                std::make_shared<WaveClipTitleEditHandle>(
                    mTextEditHelper, track)
            )
        );
    }

    const auto waveTrack = std::static_pointer_cast<WaveTrack>(track->SubstitutePendingChangedTrack());
    auto& zoomInfo = ViewInfo::Get(*pProject);
    const auto intervals = waveTrack->Intervals();
    for(auto it = intervals.begin(); it != intervals.end(); ++it)
    {
        if (it == mEditedInterval)
            continue;

        const auto clip = (*it)->GetClip(0);
        if (LowlitClipButton::HitTest<ClipButtonId::Overflow>(
               { *clip, zoomInfo, rect }, mousePoint))
        {
           results.push_back(AssignUIHandlePtr(
              mOverflowButtonHandle, std::make_shared<ClipOverflowButtonHandle>(
                                        track, *it, weak_from_this())));
           mFocusInterval = it;
           break;
        }
        else if (LowlitClipButton::HitTest<ClipButtonId::Pitch>(
                    { *clip, zoomInfo, rect }, mousePoint))
        {
           results.push_back(AssignUIHandlePtr(
              mPitchButtonHandle,
              std::make_shared<ClipPitchAndSpeedButtonHandle>(
                 ClipPitchAndSpeedButtonHandle::Type::Pitch, track, *it)));
           mFocusInterval = it;
           break;
        }
        else if (LowlitClipButton::HitTest<ClipButtonId::Speed>(
                    { *clip, zoomInfo, rect }, mousePoint))
        {
           results.push_back(AssignUIHandlePtr(
              mSpeedButtonHandle,
              std::make_shared<ClipPitchAndSpeedButtonHandle>(
                 ClipPitchAndSpeedButtonHandle::Type::Speed, track, *it)));
           mFocusInterval = it;
           break;
        }
        else if (WaveChannelView::HitTest(
                    *clip, zoomInfo, state.rect, mousePoint))
        {
           results.push_back(AssignUIHandlePtr(
              mAffordanceHandle,
              std::make_shared<WaveTrackAffordanceHandle>(track, clip)));
           mFocusInterval = it;
           break;
        }
    }

    const auto& settings = ProjectSettings::Get(*pProject);
    const auto currentTool = settings.GetTool();
    if (currentTool == ToolCodes::multiTool || currentTool == ToolCodes::selectTool)
    {
        results.push_back(
            SelectHandle::HitTest(
                mSelectHandle, state, pProject,
                ChannelView::Get(*track).shared_from_this()
            )
        );
    }

    return results;
}

void WaveTrackAffordanceControls::Draw(TrackPanelDrawingContext& context, const wxRect& rect, unsigned iPass)
{
    if (iPass == TrackArtist::PassBackground) {
        auto track = FindTrack();
        const auto artist = TrackArtist::Get(context);

        TrackArt::DrawBackgroundWithSelection(context, rect, track.get(), artist->blankSelectedBrush, artist->blankBrush);

        mVisibleIntervals.clear();

        const auto waveTrack = std::static_pointer_cast<WaveTrack>(track->SubstitutePendingChangedTrack());
        const auto& zoomInfo = *artist->pZoomInfo;
        {
            wxDCClipper dcClipper(context.dc, rect);

            context.dc.SetTextBackground(wxTransparentColor);
            context.dc.SetTextForeground(theTheme.Colour(clrClipNameText));
            context.dc.SetFont(mClipNameFont);

            auto px = context.lastState.m_x;
            auto py = context.lastState.m_y;

            const auto overflowHandle = mOverflowButtonHandle.lock();
            const auto intervals = waveTrack->Intervals();
            for(auto it = intervals.begin(); it != intervals.end(); ++it)
            {
                auto interval = *it;
                const auto& clip = *interval->GetClip(0);
                const auto clipRect = ClipParameters::GetClipRect(
                   clip, zoomInfo, rect);

                if(!WaveChannelView::ClipDetailsVisible(clip, zoomInfo, rect))
                {
                   TrackArt::DrawClipFolded(context.dc, clipRect);
                   continue;
                }

                const auto selected = GetSelectedInterval() == it;
                const auto highlightAffordance =
                    !overflowHandle && (selected || clipRect.Contains(px, py));
                auto affordanceRect = TrackArt::DrawClipAffordance(
                   context.dc, clipRect, highlightAffordance, selected);

                if (
                   const auto overflowButtonRect =
                      LowlitClipButton::DrawOnClip<ClipButtonId::Overflow>(
                         { clip, zoomInfo, rect }, context.dc))
                   affordanceRect.width -= overflowButtonRect->width;
                if (
                   const auto speedButtonRect =
                      LowlitClipButton::DrawOnClip<ClipButtonId::Speed>(
                         { clip, zoomInfo, rect }, context.dc))
                   affordanceRect.width -= speedButtonRect->width;
                if (
                   const auto pitchButtonRect =
                      LowlitClipButton::DrawOnClip<ClipButtonId::Pitch>(
                         { clip, zoomInfo, rect }, context.dc))
                   affordanceRect.width -= pitchButtonRect->width;

                if (mTextEditHelper && mEditedInterval == it)
                {
                   if (!mTextEditHelper->Draw(context.dc, affordanceRect))
                   {
                      mTextEditHelper->Cancel(nullptr);
                      TrackArt::DrawClipTitle(
                         context.dc, affordanceRect, interval->GetName());
                   }
                }
                else if (TrackArt::DrawClipTitle(
                            context.dc, affordanceRect, interval->GetName()))
                   mVisibleIntervals.push_back(it);
            }
        }

    }
}

bool WaveTrackAffordanceControls::IsIntervalVisible(const IntervalIterator& it) const noexcept
{
    return std::find(mVisibleIntervals.begin(),
                     mVisibleIntervals.end(),
                     it) != mVisibleIntervals.end();
}

bool WaveTrackAffordanceControls::StartEditClipName(AudacityProject& project, IntervalIterator it)
{
    bool useDialog{ false };
    gPrefs->Read(wxT("/GUI/DialogForNameNewLabel"), &useDialog, false);

    auto interval = *it;
    if(!interval)
        return false;

    if (useDialog)
    {
        SetWaveClipNameCommand Command;
        auto oldName = interval->GetName();
        Command.mName = oldName;
        auto result = Command.PromptUser(&GetProjectFrame(project));
        if (result && Command.mName != oldName)
        {
            interval->SetName(Command.mName);
            ProjectHistory::Get(project).PushState(XO("Modified Clip Name"),
                XO("Clip Name Edit"));
        }
    }
    else if(it != mEditedInterval)
    {
        if(!IsIntervalVisible(it))
           return false;

        if (mTextEditHelper)
            mTextEditHelper->Finish(&project);

        mEditedInterval = it;
        mTextEditHelper = MakeTextEditHelper(interval->GetName());
    }

    return true;
}

auto WaveTrackAffordanceControls::GetSelectedInterval() const -> IntervalIterator
{
    if (auto handle = mAffordanceHandle.lock())
    {
        return handle->Clicked() ? mFocusInterval : IntervalIterator{};
    }
    return {};
}

namespace {

auto FindAffordance(WaveTrack &track)
{
   auto &view = ChannelView::Get(track);
   auto pAffordance = view.GetAffordanceControls();
   return std::dynamic_pointer_cast<WaveTrackAffordanceControls>(
      pAffordance );
}

std::pair<std::shared_ptr<WaveTrack>, ChannelGroup::IntervalIterator<WaveTrack::Interval>>
SelectedIntervalOfFocusedTrack(AudacityProject &project, bool wholeInterval = true)
{
   // Note that TrackFocus may change its state as a side effect, defining
   // a track focus if there was none
   auto track = TrackFocus::Get(project).Get();
   if (!track)
      return {};
   if (
      auto pWaveTrack =
         std::dynamic_pointer_cast<WaveTrack>(track->shared_from_this()))
   {
      if (FindAffordance(*pWaveTrack)) {
         auto &viewInfo = ViewInfo::Get(project);
         auto intervals = pWaveTrack->Intervals();

         auto it = std::find_if(
            intervals.begin(), intervals.end(), [&](const auto& interval) {
               if (wholeInterval)
                  return interval->Start() == viewInfo.selectedRegion.t0() &&
                         interval->End() == viewInfo.selectedRegion.t1();
               else
                  return interval->Start() <= viewInfo.selectedRegion.t0() &&
                         interval->End() > viewInfo.selectedRegion.t0();
            });

         if(it != intervals.end())
            return { pWaveTrack, it };
      }
   }
   return {};
}

// condition for enabling the command
const ReservedCommandFlag &SomeClipIsSelectedFlag()
{
   static ReservedCommandFlag flag{
      [](const AudacityProject &project){
         // const_cast isn't pretty but not harmful in this case
         return SelectedIntervalOfFocusedTrack(const_cast<AudacityProject&>(project)).first != nullptr;
      }
   };
   return flag;
}

const ReservedCommandFlag &StretchedClipIsSelectedFlag()
{
   static ReservedCommandFlag flag{
      [](const AudacityProject &project){
         // const_cast isn't pretty but not harmful in this case
         auto result = SelectedIntervalOfFocusedTrack(
            const_cast<AudacityProject&>(project));

         auto interval = *result.second;
         return interval != nullptr && interval->HasPitchOrSpeed();
      }
   };
   return flag;
}

}

unsigned WaveTrackAffordanceControls::CaptureKey(wxKeyEvent& event, ViewInfo& viewInfo, wxWindow* pParent, AudacityProject* project)
{
    if (!mTextEditHelper
       || !mTextEditHelper->CaptureKey(event.GetKeyCode(), event.GetModifiers()))
       // Handle the event if it can be processed by the text editor (if any)
       event.Skip();
    return RefreshCode::RefreshNone;
}


unsigned WaveTrackAffordanceControls::KeyDown(wxKeyEvent& event, ViewInfo& viewInfo, wxWindow*, AudacityProject* project)
{
    auto keyCode = event.GetKeyCode();

    if (mTextEditHelper)
    {
       if (!mTextEditHelper->OnKeyDown(keyCode, event.GetModifiers(), project)
          && !TextEditHelper::IsGoodEditKeyCode(keyCode))
          event.Skip();

       return RefreshCode::RefreshCell;
    }
    return RefreshCode::RefreshNone;
}

unsigned WaveTrackAffordanceControls::Char(wxKeyEvent& event, ViewInfo& viewInfo, wxWindow* pParent, AudacityProject* project)
{
    if (mTextEditHelper && mTextEditHelper->OnChar(event.GetUnicodeKey(), project))
        return RefreshCode::RefreshCell;
    return RefreshCode::RefreshNone;
}

unsigned WaveTrackAffordanceControls::LoseFocus(AudacityProject *)
{
   return ExitTextEditing();
}

void WaveTrackAffordanceControls::OnTextEditFinished(AudacityProject* project, const wxString& text)
{
    if (auto interval = *mEditedInterval)
    {
        if (text != interval->GetName()) {
            interval->SetName(text);

            ProjectHistory::Get(*project).PushState(XO("Modified Clip Name"),
                XO("Clip Name Edit"));
        }
    }
    ResetClipNameEdit();
}

void WaveTrackAffordanceControls::OnTextEditCancelled(AudacityProject* project)
{
    ResetClipNameEdit();
}

void WaveTrackAffordanceControls::OnTextModified(AudacityProject* project, const wxString& text)
{
    //Nothing to do
}

void WaveTrackAffordanceControls::OnTextContextMenu(AudacityProject* project, const wxPoint& position)
{
}

void WaveTrackAffordanceControls::ResetClipNameEdit()
{
    mTextEditHelper.reset();
    mEditedInterval = {};
}

void WaveTrackAffordanceControls::OnTrackListEvent(const TrackListEvent& evt)
{
    if (evt.mType == TrackListEvent::SELECTION_CHANGE)
       ExitTextEditing();
}

void WaveTrackAffordanceControls::OnSelectionChange(NotifyingSelectedRegionMessage)
{
    ExitTextEditing();
}

unsigned WaveTrackAffordanceControls::ExitTextEditing()
{
    using namespace RefreshCode;
    if (mTextEditHelper)
    {
        if (auto trackList = FindTrack()->GetOwner())
        {
            mTextEditHelper->Finish(trackList->GetOwner());
        }
        ResetClipNameEdit();
        return RefreshCell;
    }
    return RefreshNone;
}

bool WaveTrackAffordanceControls::OnTextCopy(AudacityProject& project)
{
   if (mTextEditHelper)
   {
      mTextEditHelper->CopySelectedText(project);
      return true;
   }
   return false;
}

bool WaveTrackAffordanceControls::OnTextCut(AudacityProject& project)
{
   if (mTextEditHelper)
   {
      mTextEditHelper->CutSelectedText(project);
      return true;
   }
   return false;
}

bool WaveTrackAffordanceControls::OnTextPaste(AudacityProject& project)
{
   if (mTextEditHelper)
   {
      mTextEditHelper->PasteSelectedText(project);
      return true;
   }
   return false;
}

bool WaveTrackAffordanceControls::OnTextSelect(AudacityProject& project)
{
   if (mTextEditHelper)
   {
      mTextEditHelper->SelectAll();
      return true;
   }
   return false;
}

unsigned WaveTrackAffordanceControls::OnAffordanceClick(const TrackPanelMouseEvent& event, AudacityProject* project)
{
    auto& viewInfo = ViewInfo::Get(*project);
    if (mTextEditHelper)
    {
        if (auto interval = *mEditedInterval)
        {
            auto affordanceRect = ClipParameters::GetClipRect(*interval->GetClip(0), viewInfo, event.rect);
            if (!affordanceRect.Contains(event.event.GetPosition()))
               return ExitTextEditing();
        }
    }
    else if (auto interval = *mFocusInterval)
    {
        if (event.event.LeftDClick())
        {
            auto affordanceRect = ClipParameters::GetClipRect(*interval->GetClip(0), viewInfo, event.rect);
            if (affordanceRect.Contains(event.event.GetPosition()) &&
                StartEditClipName(*project, mFocusInterval))
            {
                event.event.Skip(false);
                return RefreshCode::RefreshCell;
            }
        }
    }
    return RefreshCode::RefreshNone;
}

void WaveTrackAffordanceControls::StartEditSelectedClipName(AudacityProject& project)
{
   auto [track, it] = SelectedIntervalOfFocusedTrack(project);
   if(track != FindTrack())
      return;
   StartEditClipName(project, it);
}

void WaveTrackAffordanceControls::StartEditSelectedClipSpeed(
   AudacityProject& project)
{
   constexpr auto wholeInterval = false;
   auto [track, it] = SelectedIntervalOfFocusedTrack(project, wholeInterval);

   if (track != FindTrack())
      return;

   auto interval = *it;

   if (!interval)
      return;

   PitchAndSpeedDialog::Get(project).Retarget(track, interval).SetFocus({});
}

void WaveTrackAffordanceControls::OnRenderClipStretching(
   AudacityProject& project) const
{
   const auto [track, it] = SelectedIntervalOfFocusedTrack(project);

   if (track != FindTrack())
      return;

   auto interval = *it;

   if (!interval || !interval->HasPitchOrSpeed())
      return;

   WaveTrackUtilities::WithClipRenderingProgress(
      [track = track, interval = interval](const ProgressReporter& progress) {
         track->ApplyPitchAndSpeed(
            { { interval->GetPlayStartTime(), interval->GetPlayEndTime() } },
            progress);
      },
      XO("Applying..."));

   ProjectHistory::Get(project).PushState(
      XO("Rendered time-stretched audio"), XO("Render"));

   WaveClipUtilities::SelectClip(project, *interval);
}

std::shared_ptr<TextEditHelper> WaveTrackAffordanceControls::MakeTextEditHelper(const wxString& text)
{
    auto helper = std::make_shared<TextEditHelper>(shared_from_this(), text, mClipNameFont);
    helper->SetTextColor(theTheme.Colour(clrClipNameText));
    helper->SetTextSelectionColor(theTheme.Colour(clrClipNameTextSelection));
    return helper;
}

auto WaveTrackAffordanceControls::GetMenuItems(
   const wxRect &rect, const wxPoint *pPosition, AudacityProject *pProject)
      -> std::vector<MenuItem>
{
   return WaveClipUtilities::GetWaveClipMenuItems();
}

// Register a menu item

namespace {

// Menu handler functions

void OnEditClipName(const CommandContext &context)
{
   auto &project = context.project;

   if(auto pWaveTrack = dynamic_cast<WaveTrack *>(TrackFocus::Get(project).Get()))
   {
      if(auto pAffordance = FindAffordance(*pWaveTrack))
      {
         pAffordance->StartEditSelectedClipName(project);
         // Refresh so the cursor appears
         TrackPanel::Get(project).RefreshTrack(pWaveTrack);
      }
   }
}

void OnChangePitchAndSpeed(const CommandContext& context)
{
   auto& project = context.project;

   if (
      auto pWaveTrack =
         dynamic_cast<WaveTrack*>(TrackFocus::Get(project).Get()))
   {
      if (auto pAffordance = FindAffordance(*pWaveTrack))
         pAffordance->StartEditSelectedClipSpeed(project);
   }
}

void OnRenderClipStretching(const CommandContext& context)
{
   auto& project = context.project;

   if (
      auto pWaveTrack =
         dynamic_cast<WaveTrack*>(TrackFocus::Get(project).Get()))
   {
      if (auto pAffordance = FindAffordance(*pWaveTrack))
         pAffordance->OnRenderClipStretching(project);
   }
}

void OnPitchShift(
   const CommandContext& context, bool up)
{
   auto [track, it] = SelectedIntervalOfFocusedTrack(context.project);
   if (!track)
      return;
   const auto interval = *it;
   if (interval->SetCentShift(interval->GetCentShift() + (up ? 100 : -100)))
      ProjectHistory::Get(context.project)
         .PushState(
            XO("Pitch Shift"), XO("Changed Pitch Shift"),
            UndoPush::CONSOLIDATE);
}

void OnPitchUp(const CommandContext& context)
{
   OnPitchShift(context, true);
}

void OnPitchDown(const CommandContext& context)
{
   OnPitchShift(context, false);
}

using namespace MenuRegistry;

// Register menu items

AttachedItem sAttachment{
   Command( L"RenameClip", XXO("&Rename Clip..."),
      OnEditClipName, SomeClipIsSelectedFlag(), wxT("Ctrl+F2") ),
   wxT("Edit/Other/Clip")
};

AttachedItem sAttachment2 {
   Command(
      L"ChangePitchAndSpeed", XXO("&Pitch and Speed..."), OnChangePitchAndSpeed,
      AlwaysEnabledFlag, Options { wxT("Ctrl+Shift+P") }),
   wxT("Edit/Other/Clip")
};

AttachedItem sAttachment3{
   Command( L"RenderPitchAndSpeed", XXO("Render Pitch and &Speed"),
      OnRenderClipStretching, StretchedClipIsSelectedFlag()),
   wxT("Edit/Other/Clip")
};

AttachedItem sAttachment4{
   Command( L"PitchUp", XXO("Pitch &Up"),
      OnPitchUp, SomeClipIsSelectedFlag(), Options{ wxT("Alt+Up") }),
   wxT("Edit/Other/Clip")
};

AttachedItem sAttachment5{
   Command( L"PitchDown", XXO("Pitch &Down"),
      OnPitchDown, SomeClipIsSelectedFlag(), Options{ wxT("Alt+Down") }),
   wxT("Edit/Other/Clip")
};
}
