/**********************************************************************

Audacity: A Digital Audio Editor

WaveformVZoomHandle.cpp

Paul Licameli split from WaveChannelVZoomHandle.cpp

**********************************************************************/


#include "WaveformVZoomHandle.h"

#include "WaveChannelVZoomHandle.h"

#include "../../../../HitTestResult.h"
#include "NumberScale.h"
#include "Prefs.h"
#include "ProjectHistory.h"
#include "../../../../RefreshCode.h"
#include "../../../../TrackPanelMouseEvent.h"
#include "WaveTrack.h"
#include "../../../../prefs/WaveformSettings.h"

WaveformVZoomHandle::WaveformVZoomHandle(
   const std::shared_ptr<WaveTrack> &pTrack, const wxRect &rect, int y)
      : mpTrack{ pTrack } , mZoomStart(y), mZoomEnd(y), mRect(rect)
{
}

WaveformVZoomHandle::~WaveformVZoomHandle() = default;

std::shared_ptr<const Channel> WaveformVZoomHandle::FindChannel() const
{
   return std::dynamic_pointer_cast<const Channel>(mpTrack.lock());
}

void WaveformVZoomHandle::Enter( bool, AudacityProject* )
{
#ifdef EXPERIMENTAL_TRACK_PANEL_HIGHLIGHTING
   mChangeHighlight = RefreshCode::RefreshCell;
#endif
}

bool WaveformVZoomHandle::HandlesRightClick()
{
   return true;
}

UIHandle::Result WaveformVZoomHandle::Click
(const TrackPanelMouseEvent &, AudacityProject *)
{
   return RefreshCode::RefreshNone;
}

UIHandle::Result WaveformVZoomHandle::Drag
(const TrackPanelMouseEvent &evt, AudacityProject *pProject)
{
   using namespace RefreshCode;
   auto pTrack = TrackList::Get( *pProject ).Lock(mpTrack);
   if (!pTrack)
      return Cancelled;
   return WaveChannelVZoomHandle::DoDrag(evt, pProject, mZoomStart, mZoomEnd, false);
}

HitTestPreview WaveformVZoomHandle::Preview
(const TrackPanelMouseState &st, AudacityProject *)
{
   return WaveChannelVZoomHandle::HitPreview(false);
}

UIHandle::Result WaveformVZoomHandle::Release
(const TrackPanelMouseEvent &evt, AudacityProject *pProject,
 wxWindow *pParent)
{
   auto pTrack = TrackList::Get( *pProject ).Lock(mpTrack);
   return WaveChannelVZoomHandle::DoRelease(
      evt, pProject, pParent, pTrack.get(), mRect,
      DoZoom, WaveformVRulerMenuTable::Instance(),
      mZoomStart, mZoomEnd);
}

UIHandle::Result WaveformVZoomHandle::Cancel(AudacityProject*)
{
   // Cancel is implemented!  And there is no initial state to restore,
   // so just return a code.
   return RefreshCode::RefreshAll;
}

void WaveformVZoomHandle::Draw(
   TrackPanelDrawingContext &context,
   const wxRect &rect, unsigned iPass )
{
   if (!mpTrack.lock()) //? TrackList::Lock()
      return;
   return WaveChannelVZoomHandle::DoDraw(
      context, rect, iPass, mZoomStart, mZoomEnd, false);
}

wxRect WaveformVZoomHandle::DrawingArea(
   TrackPanelDrawingContext &,
   const wxRect &rect, const wxRect &panelRect, unsigned iPass )
{
   return WaveChannelVZoomHandle::DoDrawingArea(rect, panelRect, iPass);
}

// ZoomKind says how to zoom.
// If ZoomStart and ZoomEnd are not equal, this may override
// the zoomKind and cause a drag-zoom-in.
void WaveformVZoomHandle::DoZoom(
   AudacityProject *pProject,
   WaveTrack *pTrack,
   WaveChannelViewConstants::ZoomActions ZoomKind,
   const wxRect &rect, int zoomStart, int zoomEnd,
   bool fixedMousePoint)
{
   using namespace WaveChannelViewConstants;
   static const float ZOOMLIMIT = 0.001f;

   int height = rect.height;
   int ypos = rect.y;

   // Ensure start and end are in order (swap if not).
   if (zoomEnd < zoomStart)
      std::swap( zoomStart, zoomEnd );

   float min, max, minBand = 0;
   const double rate = pTrack->GetRate();
   const float halfrate = rate / 2;
   float maxFreq = 8000.0;


   float top=2.0;
   float half=0.5;

   {
      auto &cache = WaveformScale::Get(*pTrack);
      cache.GetDisplayBounds(min, max);
      auto &waveSettings = WaveformSettings::Get(*pTrack);
      const bool linear = waveSettings.isLinear();
      if( !linear ){
         top = (LINEAR_TO_DB(2.0) + waveSettings.dBRange) / waveSettings.dBRange;
         half = (LINEAR_TO_DB(0.5) + waveSettings.dBRange) / waveSettings.dBRange;
      }
   }


   // Compute min and max.
   switch(ZoomKind)
   {
   default:
      // If we have covered all the cases, this won't happen.
      // In release builds Audacity will ignore the zoom.
      wxFAIL_MSG("Zooming Case not implemented by Audacity");
      break;
   case kZoomReset:
   case kZoom1to1:
      {
         // Zoom out full
         min = -1.0;
         max = 1.0;
      }
      break;
   case kZoomHalfWave:
      {
         // Zoom to show fractionally more than the top half of the wave.
         min = -0.01f;
         max = 1.0;
      }
      break;
   case kZoomInByDrag:
      {
         const float tmin = min, tmax = max;
         const float p1 = (zoomStart - ypos) / (float)height;
         const float p2 = (zoomEnd - ypos) / (float)height;
         max = (tmax * (1.0 - p1) + tmin * p1);
         min = (tmax * (1.0 - p2) + tmin * p2);

         // Waveform view - allow zooming down to a range of ZOOMLIMIT
         if (max - min < ZOOMLIMIT) {     // if user attempts to go smaller...
            float c = (min + max) / 2;    // ...set centre of view to centre of dragged area and top/bottom to ZOOMLIMIT/2 above/below
            min = c - ZOOMLIMIT / 2.0;
            max = c + ZOOMLIMIT / 2.0;
         }
      }
      break;
   case kZoomIn:
      {
         const float zoomFactor = 0.5f;
         const float currentRange = max - min;
         const float nextRange = std::max(zoomFactor * currentRange, ZOOMLIMIT);

         const float center = min + (currentRange / 2.0);
         min = center - (nextRange / 2.0);
         max = center + (nextRange / 2.0);
      }
      break;
   case kZoomOut:
      {
         const float zoomFactor = 2.0f;
         const float currentRange = max - min;
         const float nextRange = zoomFactor * currentRange;

         const float center = min + (currentRange / 2.0);
         min = std::max(-top, center - (0.5f * nextRange));
         max = std::min(top, center + (0.5f * nextRange));
      }
      break;
   }

   // Now actually apply the zoom.
   WaveformScale::Get(*pTrack).SetDisplayBounds(min, max);

   zoomEnd = zoomStart = 0;
   if( pProject )
      ProjectHistory::Get( *pProject ).ModifyState(true);
}

///////////////////////////////////////////////////////////////////////////////
// Table class

PopupMenuTable &WaveformVRulerMenuTable::Instance()
{
   static WaveformVRulerMenuTable instance;
   return instance;
}

BEGIN_POPUP_MENU(WaveformVRulerMenuTable)
   // this generates the linear(amp), log(dB), linear(dB) entries
   const auto & names = WaveformSettings::GetScaleNames();
   for (int ii = 0, nn = names.size(); ii < nn; ++ii) {
      AppendRadioItem( names[ii].Internal(),
         OnFirstWaveformScaleID + ii, names[ii].Msgid(),
         POPUP_MENU_FN( OnWaveformScaleType ),
         []( PopupMenuHandler &handler, wxMenu &menu, int id ){
            const auto pData =
               static_cast< WaveformVRulerMenuTable& >( handler ).mpData;
            WaveTrack *const wt = pData->pTrack;
            if ( id ==
               OnFirstWaveformScaleID +
               static_cast<int>(WaveformSettings::Get(*wt).scaleType) )
               menu.Check(id, true);
         }
         );
   }

   BeginSection( "Zoom" );
      BeginSection( "Basic" );
         AppendItem( "In", OnZoomInVerticalID, XXO("Zoom In"), POPUP_MENU_FN( OnZoomInVertical ) );
         AppendItem( "Out", OnZoomOutVerticalID, XXO("Zoom Out"), POPUP_MENU_FN( OnZoomOutVertical ) );
         AppendItem( "Reset", OnZoomFitVerticalID, XXO("Reset Zoom"), POPUP_MENU_FN( OnZoomReset ) );
      EndSection();

   #ifdef EXPERIMENTAL_HALF_WAVE
      BeginSection( "InOut" );
         AppendItem( "HalfWave", OnZoomHalfWaveID, XXO("Half Wave"), POPUP_MENU_FN( OnZoomHalfWave ) );
      EndSection();
   #endif
   EndSection();

END_POPUP_MENU()

void WaveformVRulerMenuTable::OnWaveformScaleType(wxCommandEvent &evt)
{
   WaveTrack *const wt = mpData->pTrack;
   // Assume linked track is wave or null
   const WaveformSettings::ScaleType newScaleType =
      WaveformSettings::ScaleType(
         std::max(0,
            std::min((int)(WaveformSettings::stNumScaleTypes) - 1,
               evt.GetId() - OnFirstWaveformScaleID
      )));

   if (WaveformSettings::Get(*wt).scaleType != newScaleType) {
      WaveformSettings::Get(*wt).scaleType = newScaleType;

      AudacityProject *const project = &mpData->project;
      ProjectHistory::Get( *project ).ModifyState(true);

      using namespace RefreshCode;
      mpData->result = UpdateVRuler | RefreshAll;
   }
}
