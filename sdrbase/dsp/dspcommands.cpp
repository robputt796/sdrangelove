#include "dsp/dspcommands.h"

MessageRegistrator DSPPing::ID("DSPPing");
MessageRegistrator DSPExit::ID("DSPExit");
MessageRegistrator DSPAcquisitionStart::ID("DSPAcquisitionStart");
MessageRegistrator DSPAcquisitionStop::ID("DSPAcquisitionStop");
MessageRegistrator DSPGetDeviceDescription::ID("DSPGetDeviceDescription");
MessageRegistrator DSPGetErrorMessage::ID("DSPGetErrorMessage");
MessageRegistrator DSPSetSource::ID("DSPSetSource");
MessageRegistrator DSPAddSink::ID("DSPAddSink");
MessageRegistrator DSPAddAudioSource::ID("DSPAddAudioSource");
MessageRegistrator DSPRemoveAudioSource::ID("DSPRemoveAudioSource");
MessageRegistrator DSPRemoveSink::ID("DSPRemoveSink");
MessageRegistrator DSPConfigureSpectrumVis::ID("DSPConfigureSpectrumVis");
MessageRegistrator DSPConfigureCorrection::ID("DSPConfigureCorrection");
MessageRegistrator DSPEngineReport::ID("DSPEngineReport");
MessageRegistrator DSPConfigureScopeVis::ID("DSPConfigureScopeVis");
MessageRegistrator DSPSignalNotification::ID("DSPSignalNotification");
MessageRegistrator DSPConfigureChannelizer::ID("DSPConfigureChannelizer");