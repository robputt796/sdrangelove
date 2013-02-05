#ifndef INCLUDE_SCOPEVIS_H
#define INCLUDE_SCOPEVIS_H

#include "samplesink.h"

class GLScope;
class MessageQueue;

class ScopeVis : public SampleSink {
public:
	enum TriggerChannel {
		TriggerFreeRun,
		TriggerChannelI,
		TriggerChannelQ
	};

	ScopeVis(GLScope* glScope = NULL);

	void configure(MessageQueue* msgQueue, TriggerChannel triggerChannel, Real triggerLevelHigh, Real triggerLevelLow);

	void feed(SampleVector::const_iterator begin, SampleVector::const_iterator end, bool firstOfBurst);
	void start();
	void stop();
	void setSampleRate(int sampleRate);
	void handleMessage(Message* cmd);

private:
	enum TriggerState {
		Untriggered,
		Triggered,
		WaitForReset
	};

	GLScope* m_glScope;
	std::vector<Complex> m_trace;
	int m_fill;
	TriggerState m_triggerState;
	TriggerChannel m_triggerChannel;
	FixReal m_triggerLevelHigh;
	FixReal m_triggerLevelLow;
	int m_sampleRate;
};

#endif // INCLUDE_SCOPEVIS_H
