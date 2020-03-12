# DigiSparkJiggle

This project was created to solve a very specific problem; computers in Department of Education Western Australia schools face an oppressive security policy. This policy forcibly logs out users after merely 15 minutes of inactivity. Previously software solutions such as Caffeine have been used, however these are easily detected and blocked by the Department's IT service provider.

Enter DigiSparkJiggler. Based on a DigiSpark, this little device presents as a normal USB mouse. Once every 60 seconds DigiSparkJiggler will move the curse one pixel to the right, and return one pixel to the left. Thus the system is fooled into thinking the user is continuously operating the machine, defeating the screen saver and the automatic logout. 

So as to not totally compromise the security of the system, DigiSparkJiggler will shut itself down after four hours. It can then be restarted by simply re-plugging the device or resetting the CPU. To make it easy to identify if the jiggler is operating, the on-board LED pulses once per second.

DigiSpark hardware is cheap and easy to obtain;

* <https://www.ebay.com.au/sch/i.html?_nkw=digispark>
* <https://www.aliexpress.com/wholesale?SearchText=digispark>

[DigiStump](https://digistump.com/), the original creators of the DigiSpark, have instructions on setting up a build environment; <https://digistump.com/wiki/digispark/tutorials/connecting>

The firmware itself uses the excellent [V-USB](https://github.com/obdev/v-usb) library by [Objective Development](http://www.obdev.at/)

If you are at a school in Western Australia which suffers from this pathetic security policy, and would like assistance in building your own DigiSparkJigglers, please feel free to contact me.
