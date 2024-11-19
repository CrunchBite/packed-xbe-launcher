# packed-xbe-launcher - A simple self-extracting launcher

packed-xbe-launcher is a tool to make self-extracting chain-loading executables for the original [Microsoft Xbox](https://en.wikipedia.org/wiki/Xbox_(console)). It is particularly useful for making [ENDGAME](https://github.com/XboxDev/endgame-exploit) payloads.

## Disclaimer

**This tool is not feature complete and is not indended to be.** There is minimal error checking during the extraction and launching process and most failure scenarios result in the xbox console being told to reboot.

If you require more polish than that you will have to add it yourself :)

## Building

To build this project a functional Xbox development environment using the official SDK (XDK) is required.

1. Put your payload contents in `packed-xbe-launcher\payload` and name your main XBE `default.xbe` in the root of the folder.

**NOTE: All XBE files in the payload must be [habibi](http://toogam.bespin.org/xboxmod/site/signxbe.htm)-signed!** Use the included xbedump.exe in the tools folder to sign. Example usage: `xbedump.exe default.xbe -habibi`

2. Open `packed-xbe-launcher.vcproj`
3. Set the Configuration to `Release_LTCG`
4. Select Build, then Rebuild
5. An ENDGAME zip and two XBEs will be output to the "output" folder. The 'signed' XBE is habibi signed for use with ENDGAME and other exploits. The 'retail' XBE will run on xbox consoles that are allready modified to run unsigned code.

## Usage

If you are making an ENDGAME playload copy the contents of the generated ENDGAME zip directory to a Xbox memory card following the [ENDGAME](https://github.com/XboxDev/endgame-exploit) readme, otherwise use the 'retail' XBE however you see fit.

# FAQ

#### Q: Why am I getting Error 21 after launching via ENDGAME?

* *A: Your XBE(s) must be signed using the [habibi](http://toogam.bespin.org/xboxmod/site/signxbe.htm) key. `xbedump` is included in the tools folder.*

#### Q: Why isnt this using the NXDK toolchain?

* *A: I am most comfortable using the official SDK (XDK). Feel free to rewrite this tool!*

#### Q: What is xUnzip? Why is most of this tool a static library??

* *A: xUnzip is a project that isn't yet ready for public release so you only get a compiled static library for now. Sorry :(*

# Authors

* CrunchBite ([BlueSky](https://bsky.app/profile/crunchbite.teamxlink.co.uk)/[Twitter](https://twitter.com/TXcrunchbite))
