"use strict";

var graph = (function() {
   let ctx, master, whiteNoiseSource, kick_osc, kick_gain, sn_lpf, sn_noiseGain, sn_osc, sn_oscGain, bp_osc1, bp_gain1, bp_osc2, bp_gain2, bp_mix, bp_delay, bp_delayFb, tk_osc, tk_gain;

   let now = function() {
      return ctx.currentTime;
   };

   let prepareHtmlElements = function() {
      var controllersDiv = document.querySelector('#controllers');

      var startAudioButton = document.createElement ('button');
      startAudioButton.appendChild (document.createTextNode('startAudio'));
      startAudioButton.onclick = function() {
         startAudio();
      }

      startAudioButton.style.background = "#FFFFFF";
      controllersDiv.appendChild (startAudioButton);

      var kick808Button = document.createElement ('button');
      kick808Button.appendChild (document.createTextNode('kick808'));
      kick808Button.onclick = function() {
         kick808();
      }

      kick808Button.style.background = "#2E78E2";
      controllersDiv.appendChild (kick808Button);

      var snare808Button = document.createElement ('button');
      snare808Button.appendChild (document.createTextNode('snare808'));
      snare808Button.onclick = function() {
         snare808();
      }

      snare808Button.style.background = "#FFEC76";
      controllersDiv.appendChild (snare808Button);

      var beepButton = document.createElement ('button');
      beepButton.appendChild (document.createTextNode('beep'));
      beepButton.onclick = function() {
         beep();
      }

      beepButton.style.background = "#DE4141";
      controllersDiv.appendChild (beepButton);

      var tickButton = document.createElement ('button');
      tickButton.appendChild (document.createTextNode('tick'));
      tickButton.onclick = function() {
         tick();
      }

      tickButton.style.background = "#4DB67D";
      controllersDiv.appendChild (tickButton);
   }

   window.addEventListener ('load', prepareHtmlElements);

   let whiteNoiseGenerator = function() {
      /* 
          A really simple white noise generator.
          Creates a buffer, fills it with random numbers and attach this buffer
          to the AudioBufferSourceNode named 'whiteNoiseSource'
      */
      
      // sr is the audio context's sample rate
      let sr = ctx.sampleRate;
      
      // numSamples is the length in sample of the buffer
      let numSamples = sr * 2.0;
      
      // numChannels is the number of channel of the buffer (here 2 because we want stereo noise)
      let numChannels = 2;
      
      // Create a buffer
      let buffer = ctx.createBuffer (numChannels, numSamples, sr);
      
      // For each channel of this buffer
      for (let c = 0; c < numChannels; ++c)
      {
          // Get the audio data for this channel
          let channelData = buffer.getChannelData (c);
          
          // For each sample in this channel data
          for (let i = 0; i < numSamples; ++i)
              // Set the sample value to a random value between -1 and 1
              channelData[i] = Math.random() * 2 - 1;
      }
      
      // Set (AudioBufferSourceNode) whiteNoiseSource's buffer to be the newly created buffer.
      whiteNoiseSource.buffer = buffer;
   };

   let keyListener = function() {
      /* 
          Bind some key presses to dynamic route calls.
      */
      
      // This is a way to know if this function already executed. We don't want HTML elements to be created
      // twice so we check if it has already been added.
      if (document.getElementById ("keysText") != undefined)
          return;
      
      // We'll use the keyboard to trigger sounds.
      // To do so, we check the key's code to know what key was pressed and then we call trigger a sound by
      // using the name of a dynamic route with parenthesis '()'. It means that we call the function corresponding
      // to the dynamic route.
      window.onkeypress = function(e) {
         if (e.code == "KeyQ")            kick808();
         else if (e.code == "KeyW")       snare808();
         else if (e.code == "KeyE")       beep();
         else if (e.code == "KeyR")       tick();
      }
      
      // Here we create a para HTML element and we add some text to it. This text will be displayed in the 
      // 'controllers' section of the generated HTML page
      var pElement = document.createElement("p");
      pElement.id = "keysText";
      pElement.appendChild (document.createTextNode("Try using the keyboard to trigger these sounds!"));
      pElement.appendChild (document.createElement("br"));
      pElement.appendChild (document.createTextNode("Key codes : KeyQ, KeyW, KeyE and KeyR."));
      document.getElementById("controllers").appendChild(pElement);
   };

   let masterSlider = function() {
      /* 
          Creates a HTML range element to control the master gain.
      */
      
      // This is a way to know if this function already executed. We don't want HTML elements to be created
      // twice so we check if it has already been added.
      if (document.getElementById ("masterSlider") != undefined)
          return;
      
      // Create a HTML input element with type range and prepare it.
      var slider = document.createElement("input");
      slider.id = "masterSlider";
      slider.setAttribute("type", "range");
      slider.setAttribute("min", "0");
      slider.setAttribute("max", "100");  // range works with integer values so we need an arbitrary scaling
      slider.style.width = "100%";
      
      // Set oninput callback to react to value changes
      slider.oninput = function() {
          // Sets the value of the gain parameter of (GainNode) master.
          // Don't forget to divide the slider's value by the appropriate factor to get a value in the range [0., 1.].
          master.gain.setValueAtTime(slider.value / 100, now());
      }
      
      // Create a label for the slider and add it to the 'controllers' section.
      var pElement = document.createElement("p");
      pElement.appendChild(document.createTextNode("Master gain :"));
      document.getElementById("controllers").appendChild(pElement);
      
      // Add the slider to the 'controllers' section.
      document.getElementById("controllers").appendChild(slider);
   };

   let startAudio = function() {
      var AudioContext = window.AudioContext || window.webkitAudioContext;

      // Context declaration (default context)
      ctx = new AudioContext();

      master = new GainNode(ctx, {
         gain: 0.5
      });

      whiteNoiseSource = new AudioBufferSourceNode(ctx, {
         detune: 0,
         loop: 1,
         loopEnd: 0,
         loopStart: 0,
         playbackRate: 1
      });

      master.connect(ctx.destination);

      keyListener();
      masterSlider();
      whiteNoiseGenerator();
      whiteNoiseSource.start (now());
   };

   let kick808 = function() {
      kick_osc = new OscillatorNode(ctx, {
         type: "sine",
         frequency: 660,
         detune: 0
      });

      kick_gain = new GainNode(ctx, {
         gain: 0
      });

      kick_osc.connect(kick_gain);
      kick_gain.connect(master);

      kick_osc.start(0);
      kick_osc.stop(now() + 1);
      kick_osc.frequency.setValueAtTime(300, now());
      kick_osc.frequency.exponentialRampToValueAtTime(30, now() + 0.1);
      kick_gain.gain.setValueAtTime(0, now());
      kick_gain.gain.linearRampToValueAtTime(0.9, now() + 0.001);
      kick_gain.gain.linearRampToValueAtTime(0, now() + 0.3);
   }

   let snare808 = function() {
      sn_lpf = new BiquadFilterNode(ctx, {
         type: "lowpass",
         Q: 0.03,
         detune: 0,
         frequency: 350,
         gain: 1
      });

      sn_noiseGain = new GainNode(ctx, {
         gain: 0
      });

      sn_osc = new OscillatorNode(ctx, {
         type: "sine",
         frequency: 660,
         detune: 0
      });

      sn_oscGain = new GainNode(ctx, {
         gain: 0
      });

      sn_lpf.connect(sn_noiseGain);
      whiteNoiseSource.connect(sn_lpf);
      sn_osc.connect(sn_oscGain);
      sn_oscGain.connect(master);
      sn_noiseGain.connect(master);

      sn_lpf.frequency.setValueAtTime(7000, now());
      sn_lpf.frequency.exponentialRampToValueAtTime(5000, now() + 0.05);
      sn_noiseGain.gain.setValueAtTime(0, now());
      sn_noiseGain.gain.linearRampToValueAtTime(0.5, now() + 0.001);
      sn_noiseGain.gain.linearRampToValueAtTime(0, now() + 0.05);
      sn_osc.start(now());
      sn_osc.stop(now() + 1);
      sn_osc.frequency.setValueAtTime(660, now());
      sn_osc.frequency.exponentialRampToValueAtTime(330, now() + 0.004);
      sn_oscGain.gain.setValueAtTime(0, now());
      sn_oscGain.gain.linearRampToValueAtTime(0.5, now() + 0.0001);
      sn_oscGain.gain.linearRampToValueAtTime(0, now() + 0.04);
   }

   let beep = function() {
      bp_osc1 = new OscillatorNode(ctx, {
         type: "sine",
         frequency: 440,
         detune: 0
      });

      bp_gain1 = new GainNode(ctx, {
         gain: 0
      });

      bp_osc2 = new OscillatorNode(ctx, {
         type: "sine",
         frequency: 660,
         detune: 0
      });

      bp_gain2 = new GainNode(ctx, {
         gain: 0
      });

      bp_mix = new GainNode(ctx, {
         gain: 1
      });

      bp_delay = new DelayNode(ctx, {
         maxDelayTime: 1,
         delayTime: 0.2
      });

      bp_delayFb = new GainNode(ctx, {
         gain: 0.1
      });

      bp_osc1.connect(bp_gain1);
      bp_osc2.connect(bp_gain2);
      bp_delay.connect(bp_delayFb);
      bp_gain1.connect(bp_mix);
      bp_gain2.connect(bp_mix);
      bp_mix.connect(bp_delay);
      bp_delayFb.connect(bp_mix);
      bp_mix.connect(master);

      bp_osc1.start(0);
      bp_osc2.start(0);
      bp_osc1.stop(now() + 1);
      bp_osc2.stop(now() + 1);
      bp_gain1.gain.setValueAtTime(0, now());
      bp_gain1.gain.linearRampToValueAtTime(1, now() + 0.02);
      bp_gain1.gain.linearRampToValueAtTime (0, now() + 0.09);
      bp_gain2.gain.setValueAtTime(0, now() + 0.08);
      bp_gain2.gain.linearRampToValueAtTime(1, now() + 0.1);
      bp_gain2.gain.linearRampToValueAtTime (0, now() + 0.17);
   }

   let tick = function() {
      tk_osc = new OscillatorNode(ctx, {
         type: "sine",
         frequency: 660,
         detune: 0
      });

      tk_gain = new GainNode(ctx, {
         gain: 0
      });

      tk_osc.connect(tk_gain);
      tk_gain.connect(master);

      tk_osc.start(now());
      tk_osc.frequency.setValueAtTime(6000, now() + 0.005);
      tk_osc.frequency.exponentialRampToValueAtTime(1500, now() + 0.007);
      tk_gain.gain.setValueAtTime(0, now() + 0.005);
      tk_gain.gain.linearRampToValueAtTime(0.6, now() + 0.01);
      tk_gain.gain.linearRampToValueAtTime(0, now() + 0.02);
   }

})();