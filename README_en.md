# ncd-bios-dumper

This program runs on the NeoGeo CD and was created with the concept of modulating the BIOS into audio signals.  
Please note that it is currently in a prototype stage with low accuracy. Consider it a "lucky if it works" situation.  
![top picture](screenshot/top-picture.jpg)  

## Before You Start

Do you really need to dump the BIOS of the Neo Geo CD?  
If you are going to use it in emulators, it is much easier to use a compatible BIOS such as UNIVERSE BIOS.  

Next, regarding CD-R reading on a real Neo Geo CD.  
I haven’t heard of any protection preventing CD-R reading on the Neo Geo CD, so it should work on any version of the console. (Talking about homebrew software. I've heard that some production versions of games are protected so that copying them won't work on the actual device, but that's not relevant in this case.)  
However, in general, reading CD-Rs requires a slightly stronger laser than pressed CDs. A worn out drive may not be able to read CD-Rs.  
**Please prepare a Neo Geo CD capable of reading CD-Rs.**  

Reference:
[The NeoGeo Development Wiki Burning CDs](https://wiki.neogeodev.org/index.php?title=Burning_CDs#Reading_problems)  

## BIOS/SAVE Dumping Procedure

1. Burn the CD-R  
Burn the CDimage/ncd_bios_dumper.iso or the .cue file from the latest Release zip onto a CD-R using CD writing software.  
2. Set up the recording environment  
Ensure you can record audio from the Neo Geo CD. For example, use an RCA-to-stereo pin plug cable (as shown in the photo below) to connect to a recording device or a PC’s Line-In port.  
![cable](screenshot/cable.jpg)  
3. Boot the Neo Geo CD from the CD-R  
It will start with a screen like the one below.  
![main screen](screenshot/main.png)  
Use left/right to switch between BIOS dump/SAVE dump/SAVE restore modes.  
Use up/down to select the audio output modulation method.  
Press the A button on "AM(High Speed)/mono" or "AM(Low Speed)/mono" to start playback. Playback time is approximately 4 minutes or 8 minutes.  
High Speed has a shorter playback time but lower stability. Low Speed is recommended.  
![record](screenshot/record.png)  
4. Record  
***The sound is not intended for human listening, so be careful not to play it through speakers.***  
Any recording device or software is fine as long as it meets the specifications below. For reference, I used Audacity on a Windows PC for recording.  
Play it once as a test and adjust the playback or recording volume so it reaches about 80% (linear) volume.  
The decoder adjusts the volume somewhat automatically, so it doesn’t need to be precise, but avoid clipping (exceeding the maximum volume during recording).  
Record in the following format:  

+ Sampling frequency (High Speed): 96kHz or higher (192kHz recommended)
+ Sampling frequency (Low Speed): 44.1kHz or higher
+ Bit depth: 16-bit
+ Format: WAVE (Linear PCM)

5. Finish Recording and Check CRC
When "FINISHED" appears at the bottom of the screen, it’s complete. The screen will look like the one below.  
Note down the value next to "CRC:".  
![finished](screenshot/finished.png)
  
6. Decode  
Use Decoder/nbd_decoder.exe from the latest release zip to decode.  

```plaintext
nbd_decoder.exe -m [mode] [in.wav] [out.bin]
  mode:am_mono_high am_mono_low
```

Select "am_mono_high" for High Speed or "am_mono_low" for Low Speed.  
After decoding, the CRC of the decoded file will be displayed, as shown below. If it matches the value shown on the Neo Geo CD screen, the decoding was successful.  
![decode](screenshot/decode.png)  
  
The output .bin file is the Neo Geo CD BIOS. Due to the poor modulation method, accuracy is low.  
If it’s still incorrect after trying Low Speed a few times or adjusting the recording setup, you might have to give up…  
  
The decoder source is available in a separate repository ([nbd-decoder](https://github.com/scrap-a/nbd-decoder)).  

## SAVE Restore Procedure

1. Overwrite the save file in the ISO
Extract CDimage/ncd_bios_dumper.iso from the latest release zip and use software that can edit ISO contents (e.g., WinISO) to overwrite SAVE.PRG in the image with the save file you want to restore.  
![WinISO](screenshot/winiso.png)
2. Burn the CD-R  
Burn the modified ncd_bios_dumper.iso or cue file onto a CD-R using CD writing software.  
3. Boot the Neo Geo CD from the CD-R  
It will start with a screen like the one below.  
![main screen](screenshot/main.png)  
Use left/right to switch to SAVE restore mode and press A to confirm.  
4. SAVE Restore Confirmation Screen
To prevent accidental overwriting of the console’s save data due to user error, a warning screen like the one below will appear.  
Follow the on-screen instructions.  
![save_restore](screenshot/save_restore.png)  
  
## Development Environment

[ngdevkit](https://github.com/dciabrin/ngdevkit) is used as the development environment.  
[ngdevkit-examples](https://github.com/dciabrin/ngdevkit-examples), sample code from ngdevkit, is also used in the source and resources.  
As a result, the license follows [ngdevkit-examples](https://github.com/dciabrin/ngdevkit-examples) (GPL3.0).  
Note that the version of ngdevkit used is old (around February 2023?), so it cannot be built with the latest ngdevkit environment. I plan to update it eventually.  

## Modulation Method

If categorized, it would fall under Amplitude Modulation (AM), but it’s such a primitive method that calling it AM feels overly generous.  
The amplitude (A1, A2, A3) of each sample (black dots in the image below) is adjusted based on the BIOS data, encoding 2 bits of information per amplitude.  
![am_method](screenshot/am_method.png)  

## ToDo

+ Migrate to the latest [ngdevkit](https://github.com/dciabrin/ngdevkit) environment  
+ Support stereo output (for faster processing)  
+ Support traditional modulation methods like KCS or SCS (for stability)

However, updates are not guaranteed—it depends on my motivation.  

## (Reference) Can It Be Used with Neo CD SD Loader?

Yes, but since [Neo CD SD Loader](http://furrtek.free.fr/sdloader/) patches the BIOS, you won’t be able to dump the correct BIOS as a result.  
To dump the BIOS using Neo CD SD Loader, refer to the following article:  (Japanese Only)
  
[Neo CD SD Loaderを使ってネオジオCDのBIOSをダンプする - 発明の友](http://blog.livedoor.jp/scrap_a/archives/36821052.html)  