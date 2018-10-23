function AudioPredecoder() {
    this.onSuccess = function() {}
    
    this.buffersToDecode = [];

    this.predecode = function() {
        for (buffer of this.buffersToDecode)
            if (buffer.shouldPredecode)
                buffer.decode();
    }

    this.audioFinishedDecoded = function() {
        if (this.buffersToDecode.length === 0)
            this.onSuccess();
    }
    
    return this;
};
       
function DecodableAudio (predecoder, ctx, useFileChooser, url, predecode) {
    this.buffer;
    this.shouldPredecode = predecode;
    let that = this;
    
    predecoder.buffersToDecode.push (this);
    
    this.decode = function() {
        getData (url);
    }
    
    function getData (url) {
        request = new XMLHttpRequest();
        request.open('GET', url, true);
        request.responseType = 'arraybuffer';
        
        request.onload = function() {
            let audioData = request.response;
            ctx.decodeAudioData(audioData,
                                onDecodeSuccess.bind (that),
                                onDecodeError.bind (that));
        }
        
        request.send();
    }
    
    function onDecodeSuccess (newBuffer) {
        this.buffer = newBuffer;
        predecoder.buffersToDecode.splice (predecoder.buffersToDecode.indexOf (this), 1);
        
        predecoder.audioFinishedDecoded();
    }
    
    function onDecodeError (e) {
        predecoder.buffersToDecode.splice (predecoder.buffersToDecode.indexOf (this), 1);
        console.log ("Error with decoding audio data" + e.error);
        
        predecoder.audioFinishedDecoded();
    }
};
