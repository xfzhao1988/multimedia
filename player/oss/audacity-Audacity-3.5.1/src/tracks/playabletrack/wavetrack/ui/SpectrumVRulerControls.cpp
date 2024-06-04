/**********************************************************************

Audacity: A Digital Audio Editor

SpectrumVRulerControls.cpp

Paul Licameli split from WaveChannelVRulerControls.cpp

**********************************************************************/

#include "SpectrumVRulerControls.h"

#include "SpectrumVZoomHandle.h"
#include "WaveChannelVRulerControls.h"

#include "../../../ui/ChannelView.h"
#include "NumberScale.h"
#include "ProjectHistory.h"
#include "../../../../RefreshCode.h"
#include "../../../../TrackPanelMouseEvent.h"
#include "WaveTrack.h"
#include "../../../../prefs/SpectrogramSettings.h"
#include "../../../../widgets/Ruler.h"
#include "../../../../widgets/LinearUpdater.h"
#include "../../../../widgets/LogarithmicUpdater.h"
#include "../../../../widgets/IntFormat.h"
#include "../../../../widgets/RealFormat.h"

SpectrumVRulerControls::~SpectrumVRulerControls() = default;

std::vector<UIHandlePtr> SpectrumVRulerControls::HitTest(
   const TrackPanelMouseState &st,
   const AudacityProject *pProject)
{
   std::vector<UIHandlePtr> results;

   if ( st.state.GetX() <= st.rect.GetRight() - kGuard ) {
      auto pTrack = FindTrack()->SharedPointer<WaveTrack>(  );
      if (pTrack) {
         auto result = std::make_shared<SpectrumVZoomHandle>(
            pTrack, st.rect, st.state.m_y );
         result = AssignUIHandlePtr(mVZoomHandle, result);
         results.push_back(result);
      }
   }

   auto more = ChannelVRulerControls::HitTest(st, pProject);
   std::copy(more.begin(), more.end(), std::back_inserter(results));

   return results;
}

unsigned SpectrumVRulerControls::HandleWheelRotation(
   const TrackPanelMouseEvent &evt, AudacityProject *pProject)
{
   using namespace RefreshCode;
   const auto pTrack = FindTrack();
   if (!pTrack)
      return RefreshNone;
   const auto wt = static_cast<WaveTrack*>(pTrack.get());
   return DoHandleWheelRotation( evt, pProject, wt );
}

unsigned SpectrumVRulerControls::DoHandleWheelRotation(
   const TrackPanelMouseEvent &evt, AudacityProject *pProject, WaveTrack *wt)
{
   using namespace RefreshCode;
   const wxMouseEvent &event = evt.event;
   
   if (!(event.ShiftDown() || event.CmdDown()))
      return RefreshNone;
   
   // Always stop propagation even if the ruler didn't change.  The ruler
   // is a narrow enough target.
   evt.event.Skip(false);
   
   auto steps = evt.steps;
   
   using namespace WaveChannelViewConstants;
   if (event.CmdDown() && !event.ShiftDown()) {
      const int yy = event.m_y;
      SpectrumVZoomHandle::DoZoom(
         pProject, wt,
         (steps < 0)
            ? kZoomOut
            : kZoomIn,
         evt.rect, yy, yy, true);
   }
   else if (!event.CmdDown() && event.ShiftDown()) {
      // Scroll some fixed number of pixels, independent of zoom level or track height:
      static const float movement = 10.0f;
      const int height = evt.rect.GetHeight();
      {
         const float delta = steps * movement / height;
         SpectrogramSettings &settings = SpectrogramSettings::Own(*wt);
         const bool isLinear = settings.scaleType == SpectrogramSettings::stLinear;
         float bottom, top;
         SpectrogramBounds::Get(*wt).GetBounds(*wt, bottom, top);
         const double rate = wt->GetRate();
         const float bound = rate / 2;
         const NumberScale numberScale(settings.GetScale(bottom, top));
         float newTop =
         std::min(bound, numberScale.PositionToValue(1.0f + delta));
         const float newBottom =
         std::max((isLinear ? 0.0f : 1.0f),
                  numberScale.PositionToValue(numberScale.ValueToPosition(newTop) - 1.0f));
         newTop =
         std::min(bound,
                  numberScale.PositionToValue(numberScale.ValueToPosition(newBottom) + 1.0f));
         
         SpectrogramBounds::Get(*wt).SetBounds(newBottom, newTop);
      }
   }
   else
      return RefreshNone;
   
   ProjectHistory::Get( *pProject ).ModifyState(true);
   
   return RefreshCell | UpdateVRuler;
}

void SpectrumVRulerControls::Draw(
   TrackPanelDrawingContext &context,
   const wxRect &rect_, unsigned iPass )
{
   ChannelVRulerControls::Draw(context, rect_, iPass);
   WaveChannelVRulerControls::DoDraw(*this, context, rect_, iPass);
}

void SpectrumVRulerControls::UpdateRuler( const wxRect &rect )
{
   const auto wt = std::static_pointer_cast< WaveTrack >( FindTrack() );
   if (!wt)
      return;
   DoUpdateVRuler( rect, wt.get() );
}

void SpectrumVRulerControls::DoUpdateVRuler(
   const wxRect &rect, const WaveTrack *wt )
{
   auto vruler = &WaveChannelVRulerControls::ScratchRuler();
   const auto &settings = SpectrogramSettings::Get(*wt);
   float minFreq, maxFreq;
   SpectrogramBounds::Get(*wt).GetBounds(*wt, minFreq, maxFreq);
   vruler->SetDbMirrorValue(0.0);
   
   switch (settings.scaleType) {
      default:
         wxASSERT(false);
      case SpectrogramSettings::stLinear:
      {
         // Spectrum
         
         /*
          draw the ruler
          we will use Hz if maxFreq is < 2000, otherwise we represent kHz,
          and append to the numbers a "k"
          */
         vruler->SetBounds(
            rect.x, rect.y, rect.x + rect.width, rect.y + rect.height - 1);
         vruler->SetOrientation(wxVERTICAL);
         vruler->SetFormat(&RealFormat::LinearInstance());
         vruler->SetLabelEdges(true);
         // use kHz in scale, if appropriate
         if (maxFreq >= 2000) {
            vruler->SetRange((maxFreq / 1000.), (minFreq / 1000.));
            /* i18n-hint k abbreviating kilo meaning thousands */
            vruler->SetUnits(XO("k"));
         }
         else {
            // use Hz
            vruler->SetRange((int)(maxFreq), (int)(minFreq));
            vruler->SetUnits({});
         }
         vruler->SetUpdater(&LinearUpdater::Instance());
      }
         break;
      case SpectrogramSettings::stLogarithmic:
      case SpectrogramSettings::stMel:
      case SpectrogramSettings::stBark:
      case SpectrogramSettings::stErb:
      case SpectrogramSettings::stPeriod:
      {
         // SpectrumLog
         
         /*
          draw the ruler
          we will use Hz if maxFreq is < 2000, otherwise we represent kHz,
          and append to the numbers a "k"
          */
         vruler->SetBounds(
            rect.x, rect.y, rect.x + rect.width, rect.y + rect.height - 1);
         vruler->SetOrientation(wxVERTICAL);
         vruler->SetFormat(&IntFormat::Instance());
         vruler->SetLabelEdges(true);
         vruler->SetRange(maxFreq, minFreq);
         vruler->SetUnits({});
         vruler->SetUpdater(&LogarithmicUpdater::Instance());
         NumberScale scale(settings.GetScale(minFreq, maxFreq).Reversal());
         vruler->SetNumberScale(scale);
      }
         break;
   }
   auto &size = ChannelView::Get(*wt).vrulerSize;
   vruler->GetMaxSize(&size.first, &size.second);
}
