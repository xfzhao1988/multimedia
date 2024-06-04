/**********************************************************************

Audacity: A Digital Audio Editor

SpectrumVZoomHandle.cpp

Paul Licameli split from WaveChannelVZoomHandle.cpp

**********************************************************************/


#include "SpectrumVZoomHandle.h"

#include "WaveChannelVZoomHandle.h"

#include "../../../../HitTestResult.h"
#include "NumberScale.h"
#include "Prefs.h"
#include "ProjectHistory.h"
#include "../../../../RefreshCode.h"
#include "../../../../TrackPanelMouseEvent.h"
#include "WaveTrack.h"
#include "../../../../prefs/SpectrogramSettings.h"

SpectrumVZoomHandle::SpectrumVZoomHandle
(const std::shared_ptr<WaveTrack> &pTrack, const wxRect &rect, int y)
   : mpTrack{ pTrack } , mZoomStart(y), mZoomEnd(y), mRect(rect)
{
}

SpectrumVZoomHandle::~SpectrumVZoomHandle() = default;

std::shared_ptr<const Channel> SpectrumVZoomHandle::FindChannel() const
{
   return mpTrack.lock();
}

void SpectrumVZoomHandle::Enter( bool, AudacityProject* )
{
#ifdef EXPERIMENTAL_TRACK_PANEL_HIGHLIGHTING
   mChangeHighlight = RefreshCode::RefreshCell;
#endif
}

bool SpectrumVZoomHandle::HandlesRightClick()
{
   return true;
}

UIHandle::Result SpectrumVZoomHandle::Click
(const TrackPanelMouseEvent &, AudacityProject *)
{
   return RefreshCode::RefreshNone;
}

UIHandle::Result SpectrumVZoomHandle::Drag
(const TrackPanelMouseEvent &evt, AudacityProject *pProject)
{
   using namespace RefreshCode;
   auto pTrack = TrackList::Get( *pProject ).Lock(mpTrack);
   if (!pTrack)
      return Cancelled;
   return WaveChannelVZoomHandle::DoDrag(evt, pProject, mZoomStart, mZoomEnd, true);
}

HitTestPreview SpectrumVZoomHandle::Preview
(const TrackPanelMouseState &st, AudacityProject *)
{
   return WaveChannelVZoomHandle::HitPreview(true);
}

UIHandle::Result SpectrumVZoomHandle::Release
(const TrackPanelMouseEvent &evt, AudacityProject *pProject,
 wxWindow *pParent)
{
   auto pTrack = TrackList::Get( *pProject ).Lock(mpTrack);
   return WaveChannelVZoomHandle::DoRelease(
      evt, pProject, pParent, pTrack.get(), mRect,
      DoZoom, SpectrumVRulerMenuTable::Instance(),
      mZoomStart, mZoomEnd);
}

UIHandle::Result SpectrumVZoomHandle::Cancel(AudacityProject*)
{
   // Cancel is implemented!  And there is no initial state to restore,
   // so just return a code.
   return RefreshCode::RefreshAll;
}

void SpectrumVZoomHandle::Draw(
   TrackPanelDrawingContext &context,
   const wxRect &rect, unsigned iPass )
{
   if (!mpTrack.lock()) //? TrackList::Lock()
      return;
   return WaveChannelVZoomHandle::DoDraw(
      context, rect, iPass, mZoomStart, mZoomEnd, true );
}

wxRect SpectrumVZoomHandle::DrawingArea(
   TrackPanelDrawingContext &,
   const wxRect &rect, const wxRect &panelRect, unsigned iPass )
{
   return WaveChannelVZoomHandle::DoDrawingArea(rect, panelRect, iPass);
}

// ZoomKind says how to zoom.
// If ZoomStart and ZoomEnd are not equal, this may override
// the zoomKind and cause a drag-zoom-in.
void SpectrumVZoomHandle::DoZoom(
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
   const auto &specSettings = SpectrogramSettings::Get(*pTrack);
   NumberScale scale;
   const bool spectrumLinear =
      (SpectrogramSettings::Get(*pTrack).scaleType == SpectrogramSettings::stLinear);

   bool bDragZoom = WaveChannelVZoomHandle::IsDragZooming(zoomStart, zoomEnd, true);
   // Add 100 if spectral to separate the kinds of zoom.
   const int kSpectral = 100;

   // Possibly override the zoom kind.
   if( bDragZoom )
      ZoomKind = kZoomInByDrag;

   float top=2.0;
   float half=0.5;

   {
      SpectrogramBounds::Get(*pTrack).GetBounds(*pTrack, min, max);
      scale = (specSettings.GetScale(min, max));
      const auto fftLength = specSettings.GetFFTLength();
      const float binSize = rate / fftLength;
      maxFreq = SpectrumMaxFreq.Read();
      // JKC:  Following discussions of Bug 1208 I'm allowing zooming in
      // down to one bin.
      //      const int minBins =
      //         std::min(10, fftLength / 2); //minimum 10 freq bins, unless there are less
      const int minBins = 1;
      minBand = minBins * binSize;
   }

   // Compute min and max.
   switch(ZoomKind)
   {
   default:
      // If we have covered all the cases, this won't happen.
      // In release builds Audacity will ignore the zoom.
      wxFAIL_MSG("Zooming Case not implemented by Audacity");
      break;

   // VZooming on spectral we don't implement the other zoom presets.
   // They are also not in the menu.
   case kZoomReset:
      {
         // Zoom out to normal level.
         min = spectrumLinear ? 0.0f : 1.0f;
         max = maxFreq;
      }
      break;
   case kZoom1to1:
   case kZoomHalfWave:
      {
         // Zoom out full
         min = spectrumLinear ? 0.0f : 1.0f;
         max = halfrate;
      }
      break;
   case kZoomInByDrag:
      {
         double xmin = 1 - (zoomEnd - ypos) / (float)height;
         double xmax = 1 - (zoomStart - ypos) / (float)height;
         const float middle = (xmin + xmax) / 2;
         const float middleValue = scale.PositionToValue(middle);

         min = std::max(spectrumLinear ? 0.0f : 1.0f,
            std::min(middleValue - minBand / 2,
            scale.PositionToValue(xmin)
            ));
         max = std::min(halfrate,
            std::max(middleValue + minBand / 2,
            scale.PositionToValue(xmax)
            ));
      }
      break;
   case kZoomIn:
      {
         // Center the zoom-in at the click
         const float p1 = (zoomStart - ypos) / (float)height;
         const float middle = 1.0f - p1;
         const float middleValue = scale.PositionToValue(middle);

         if (fixedMousePoint) {
            min = std::max(spectrumLinear ? 0.0f : 1.0f,
               std::min(middleValue - minBand * middle,
               scale.PositionToValue(0.5f * middle)
            ));
            max = std::min(halfrate,
               std::max(middleValue + minBand * p1,
               scale.PositionToValue(middle + 0.5f * p1)
            ));
         }
         else {
            min = std::max(spectrumLinear ? 0.0f : 1.0f,
               std::min(middleValue - minBand / 2,
               scale.PositionToValue(middle - 0.25f)
            ));
            max = std::min(halfrate,
               std::max(middleValue + minBand / 2,
               scale.PositionToValue(middle + 0.25f)
            ));
         }
      }
      break;
   case kZoomOut:
      {
         // Zoom out
         const float p1 = (zoomStart - ypos) / (float)height;
         // (Used to zoom out centered at midline, ignoring the click, if linear view.
         //  I think it is better to be consistent.  PRL)
         // Center zoom-out at the midline
         const float middle = // spectrumLinear ? 0.5f :
            1.0f - p1;

         if (fixedMousePoint) {
            min = std::max(spectrumLinear ? 0.0f : 1.0f, scale.PositionToValue(-middle));
            max = std::min(halfrate, scale.PositionToValue(1.0f + p1));
         }
         else {
            min = std::max(spectrumLinear ? 0.0f : 1.0f, scale.PositionToValue(middle - 1.0f));
            max = std::min(halfrate, scale.PositionToValue(middle + 1.0f));
         }
      }
      break;
   }

   // Now actually apply the zoom.
   SpectrogramBounds::Get(*pTrack).SetBounds(min, max);

   zoomEnd = zoomStart = 0;
   if( pProject )
      ProjectHistory::Get( *pProject ).ModifyState(true);
}

///////////////////////////////////////////////////////////////////////////////
// Table class

PopupMenuTable &SpectrumVRulerMenuTable::Instance()
{
   static SpectrumVRulerMenuTable instance;
   return instance;
}

BEGIN_POPUP_MENU(SpectrumVRulerMenuTable)
   //Generate scales (Mel, Logarithmic, etc)
   const auto & names = SpectrogramSettings::GetScaleNames();
   for (int ii = 0, nn = names.size(); ii < nn; ++ii) {
      AppendRadioItem( names[ii].Internal(),
         OnFirstSpectrumScaleID + ii, names[ii].Msgid(),
         POPUP_MENU_FN( OnSpectrumScaleType ),
         []( PopupMenuHandler &handler, wxMenu &menu, int id ){
            WaveTrack *const wt =
               static_cast<SpectrumVRulerMenuTable&>( handler )
                  .mpData->pTrack;
            if ( id ==
               OnFirstSpectrumScaleID +
                     static_cast<int>(SpectrogramSettings::Get(*wt).scaleType))
               menu.Check(id, true);
         }
      );
   }


BeginSection( "Zoom" );
   AppendItem( "In", OnZoomInVerticalID, XXO("Zoom In"),POPUP_MENU_FN( OnZoomInVertical ) );
   AppendItem( "Out", OnZoomOutVerticalID, XXO("Zoom Out"), POPUP_MENU_FN( OnZoomOutVertical ) );
   AppendItem( "Fit", OnZoomFitVerticalID, XXO("Zoom to Fit"), POPUP_MENU_FN( OnZoomFitVertical ) );
   AppendItem( "Reset", OnZoomResetID, XXO("Reset Zoom"),POPUP_MENU_FN( OnZoomReset ) );
EndSection();

END_POPUP_MENU()

void SpectrumVRulerMenuTable::OnSpectrumScaleType(wxCommandEvent &evt)
{
   WaveTrack *const wt = mpData->pTrack;

   const SpectrogramSettings::ScaleType newScaleType =
      SpectrogramSettings::ScaleType(
         std::max(0,
            std::min((int)(SpectrogramSettings::stNumScaleTypes) - 1,
               evt.GetId() - OnFirstSpectrumScaleID
      )));
   if (SpectrogramSettings::Get(*wt).scaleType != newScaleType) {
      SpectrogramSettings::Own(*wt).scaleType = newScaleType;

      ProjectHistory::Get( mpData->project ).ModifyState(true);

      using namespace RefreshCode;
      mpData->result = UpdateVRuler | RefreshAll;
   }
}
