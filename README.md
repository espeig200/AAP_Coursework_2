The PingPongaLong mk1

Overview
The plugin is a VST3 ping pong delay built in C++ utilising JUCE.
With a delay time dial, a feedback dial, and a dry/wet mix slider, a variation of ping pong effects can be achieved.
The PingPongaLong mk1 also includes a bpm synchronisation function with 5 settings (1/4, 1/8, 1/8T, 1/16, and 1/16T) adding a greater use range for the plugin.

Instructions:
Delay time (dT) (10-3000) can be controlled using the left hand dial.
Feedback amount (F) (0-0.99) can be controlled using the right hand dial.
Below these is found the Dry/Wet Mixer which ranges from 0-1.

Further down again, there is tick box labelled 'Sync to Tempo'
When this is checked the plugin will run the BPM sync algorithm as opposed to the base Ping Pong.
next to this box there is a drop down menu to be accessed listing the optional beat divisions.


Algorithms
the plugin utilises 2 main algorithms:
- Ping Pong Delay
- BPM Sync
 
Ping Pong Algorithm
As this is a stereo plugin, the algorithm initially checks if there are at least 2 channels in order for it to run.
Next, it looks for pointers to the audio buffer and delay buffer
- 'channelDataL' and 'channelDataR' are pointers to the left and right audio channels in the current audio buffer (the input and output audio for the plugin).
- 'delayDataL' and 'delayDataR' are pointers to the left and right delay buffers, where the delayed audio samples are stored.
The algorithm then processes each sample in the audio buffer in turn with 'numSamples' representing how many samples are currently in the audio buffer.
These processed samples are written into the delay buffer at the 'writeposition'.
by using a modulo operator, it is ensured the write position successfully wraps back round to the beginning of the delay buffer when the end is reached.
Using 'inputSample' to hold the current left channel sample, 'delayedSampleL' and 'delayedSampleR' holding delayed samples from their respective channels.
'inL' and 'inR' are both original input samples and are required later for the dry/wet mix.
Next, the delay buffer is written with feedback meaning that the new samples written are a combination of the input and a previous delayed sample.
For the left channel, the new sample is the input plus some of the right channel's delayed sample, then multiplied by 'mFeedback'.
For the right, only the left channels delayed sample is used and multiplied by 'mFeedback'.
'mFeedback' controls how much of the delayed signal is written back into the delay buffer while also creating a feedback loop, causing the delay effect to repeat over and over.
With this complete, the ping pong can be deployed.
The left channel's delay uses the right channel's delayed signal and vice versa. So, the left and right channels affect each other in the feedback loop, creating a ping-pong effect.
The logic for this is very basic:
    float wetL = delayedSampleL;
    float wetR = delayedSampleR;
The penultimate aspect of the algorithm is the Dry/Wet mix.
The dry signal ('inL' and 'inR') is mixed with the wet signal ('wetL' and 'wetR') using the dryWet parameter.
    channelDataL[i] = inL * (1.0f - dryWet) + wetL * dryWet;  // Dry/Wet mix for left channel
    channelDataR[i] = inR * (1.0f - dryWet) + wetR * dryWet;  // Dry/Wet mix for right channel
This is linked to a horizontal slider on the UI allowing it to be manipulated as with delay time and feedback.
Finally, the algorithm finishes off by moving the write position forwards.
Once proccessing is finished, the 'writeposition' is updated by the number of samples it has just completed.
Much like earlier in the algorithm, the modulo operator is used to wrap back round should it exceed the size of the delay buffer.

BPM Sync Algorithm
Utilising 'tempoSync', the plugin knows to synchronise its effects (i.e. delay time) to the current BPM of the session with it acquiring this value from the parameter system '(treeState)'.
'divisionIndex' provides integers to define how the plugin responds to note division. It corresponds to what subdivision of a beat it is synchronising to.
By employing 'getPlayHead()' the plugin can identify data about the session relevant to the plugin such as bpm, time position etc.
Once the relevant information is gathered, if '(tempoSync)' is used to check if BPM sync is on or if the plugin should use its default algorithm.
If synchronisation is on, 'beatLengthMS' calculates the duration of a beat in ms by using 60000.0f / currentBPM.
This gives 'delayMS' a default value of one beat or a quarter note (1/4).
By dividing this value, the plugin is successfully able to sync to the subdivisions shown below: 
    case 0: delayMs = beatLengthMs; break;             // 1/4
    case 1: delayMs = beatLengthMs / 2.0f; break;      // 1/8
    case 2: delayMs = beatLengthMs / 3.0f; break;      // 1/8T
    case 3: delayMs = beatLengthMs / 4.0f; break;      // 1/16
    case 4: delayMs = beatLengthMs / 6.0f; break;      // 1/16T
The use of 'switch (divisionIndex)' allows switching between the subdivisions with 'delayMS' adjusted accordingly.
Finally, by declaring 'mDelayLine = delayMs' the algorithm overrides the delay from the base ping pong algorithm allowing it to successfully sync.

The Advanced Task
For the advanced task, a BPM sync was implemented which has been laid out in great detail above.
It involved setting up UI checkboxes and menus, mathematical conversions, and a way to switch between algorithms.
The addition of BPM sync to a plugin such as The PingPongaLong mk1 allows it a much greater use range and diversity in how it is deployed as it expands the available possibilities for its use in music production.

Bugs and Issues
There is only one known bug with The PingPongaLong mk1.
Upon initially adding the plugin, the Reaper track mutes itself as it believes it will clip.
This can be easily overcome by adjusting the Dry/Wet mix and the issue resolves itself.

Further Work
There are 2 aspects that require further development:
- Gain Staging
- Parameter outputs
Gain staging would be useful to the project as it would likely resolve the bug currently being experienced in reaper.
It would also allow greater creative control over the plugin improve ease of use.
By adding parameter outputs it would allow the users to accurately identify the delay and feedback amounts applied.
Implementing these was attempted during the project, but despite several attempts, they always remained static and unchanging which effectively stripped any worth from them.


