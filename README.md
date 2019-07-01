# WebAudio Visual Editor

**Important note : this project needs heavy refactoring. It was developed on OSX and there are many unsolved issues on Windows. It's read-only for now. A lot of GUI parts will be turned into separate JUCE modules for more maintainability and easier debugging when working on WebAudio Visual Editor's specific code.**

![WebAudio Visual Editor screenshot](/docs/images/screenshot01.png)

WebAudio Visual Editor is an open-source project that aims at giving the HTML5 WebAudio API a powerful GUI, in the form of a desktop software written in C++ with JUCE. It generates vanilla Javascript code.

WebAudio Visual Editor **is not** :
* A HTML5 app running in the browser. It is a software running on desktop (tested on OSX and Windows) that generates HTML5 code (that is HTML, Javascript and CSS).
* A Javascript library. If WebAudio Visual Editor comes with some JS code, it's not designed to be used as a library but as a source code generator.
* A digital audio workstation. In fact, WebAudio Visual Editor doesn't even produce sound by itself. Instead, it uses a web browser to generate sound. It makes it easy to produce sound making virtual devices and deploy them on websites.

Here are a few design goals :
* **Visual editing** : audio graphs can be edited in a modular view. Other elements can be added to the graph such as dynamic containers, messages to nodes, comments or JS sub-scripts. The resulting JS code is generated accordingly as well as a HTML test project.
* **Reusability** : graph elements can be saved as presets. This makes it easy to share the most common portions of code between projects and prepare templates for complex tasks.
* Produce code with **no dependency** : you can just copy and paste the few JS files generated with WebAudio Visual Editor into your existing project without worrying about including and maintaining other externals. The output code is vanilla Javascript and only uses code defined in the WebAudio API. It is encapsulated using the _module_ pattern so that it won't interfere with other Javascript code.
* Designed as a **playground** : the software is primarily made to explore sound generation techniques and quickly prototype ideas. It is meant to be an invitation for non-specialists to get to know the WebAudio API. Working project examples are included and testing a project in the browser is as simple as hitting a button! If only a few things can be achieved without typing Javascript code, the software aims at providing the user with suggestions and making easier tedious tasks such as loading an audio file by using a few higher-level objects.

Please note that for now, the software is more suited to prototyping than to 'serious' web production. Most notably, the software is based on the latest [WebAudio API w3c spec](https://www.w3.org/TR/webaudio/) which is not fully implemented in all browsers. You can have more info on supported platforms on [Mozilla's guide](https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API).

## Getting Started

For OSX, you can get the latest built version [here](https://pckerneis.com/upload/WebAudioVisualEditor_0.0.1_OSX.dmg). Simply download and open the DMG file and copy the app somewhere on your computer and you're done.

For Windows and Linux users or to prepare a development version, you'll find instructions below to build WebAudio Visual Editor from a Projucer file.

### Prerequisites

You'll need the library code from JUCE. You can get it on their [website](https://shop.juce.com/get-juce) or on [GitHub](https://github.com/WeAreROLI/JUCE). We also use the [Mongoose module](https://github.com/cpenny42/juce_mongoose) for JUCE.

### Installation

Open "_WebAudio Visual Editor.jucer_" with Projucer, set the exporter for your development environment and generate the project. Build and run from your development environment. If you need more information for this step, here is a [complete tutorial](https://docs.juce.com/master/tutorial_new_projucer_project.html).

### Testing

You can then open one of the example files and hit the "_Test in browser_" button. This should open a new tab in your default browser with a test page containing at least the name of the project and a "startAudio" button. If everything works, you should be able to hear the sounds from the examples.

## Built With

* [JUCE](https://juce.com/) - Cross-platform C++ library
* [Mongoose module for JUCE](https://github.com/cpenny42/juce_mongoose) - A JUCE wrapper for [Mongoose](https://github.com/cesanta/mongoose) embedded web server

## Contributing

Feel free to submit contributions. For code styling, please refer to [JUCE's coding standards](https://juce.com/discover/stories/coding-standards).

## Authors

* **Pierre-Clément Kerneïs** - *Initial work* - [pckerneis](https://github.com/pckerneis)

See also the list of [contributors](https://github.com/pckerneis/WebAudioVisualEditor/contributors) who participated in this project.

## License

This project is licensed under the GPL v3 License - see the [LICENSE](LICENSE) file for details
