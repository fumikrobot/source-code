# Fumik robot

In August 2021, I got an idea to make a robot be able to draw pictures on a wall. After many days researching, designing, and programming, I finally finished a robot that could draw any pattern on a wall.

![](https://github.com/fumikrobot/NonCodeFiles/blob/main/1.jpg?raw=true)


My key-point in design is using general parts to make robot, so end-users can easily renew parts if repairing is needed. Body frame, motor, bolts and nuts, … are all easily found on the market. Main controller is well-known PCB Arduino Mega compatible with CNC Shield to control two stepper motors that allow robot can traverse by belt.

![](https://github.com/fumikrobot/NonCodeFiles/blob/main/2.jpg?raw=true)

Fumik is now ready for you to hang it on your house wall and start drawing any pictures. You can use Fumik to customize your house with decor picture when you wish; make your house to be special in case of a Christmas, birthday or an anniversary!


This project provides a base open-source platform to create amazing programmable gaits, locomotion, and deployment of inverse kinematics quadruped robots and bring simulations to the real world via C/C++/Python programming languages.  Our users have deployed [NVIDIA Issac simulations and reinforcement learning on our robots](https://www.youtube.com/playlist?list=PLHMFXft_rV6MWNGyofDzRhpatxZuUZMdg). Our users have also successfully deployed OpenCat on their DIY 3D-print robot pets.

![](https://github.com/PetoiCamp/NonCodeFiles/blob/master/gif/stand.gif?raw=true)

![](https://github.com/PetoiCamp/NonCodeFiles/blob/master/gif/NybbleBalance.gif?raw=true)

We've successfully crowdfunded these two mini robot kits and shipped thousands of units worldwide.  With our customized Arduino board and servos coordinating all instinctive and sophisticated movements(walking, running, jumping, backflipping), one can clip on various sensors to bring in perception and inject artificial intelligence capabilities by mounting a Raspberry Pi or other AI chips(such as Nvidia Jetson Nano) through wired/wireless connections.  Please see [Petoi FAQs](https://www.petoi.com/pages/faq?utm_source=github&utm_medium=code&utm_campaign=faq) for more info.

Also, Check out [all of the OpenCat and Petoi robot user showcases](https://www.petoi.camp/forum/showcase).


![](https://github.com/PetoiCamp/NonCodeFiles/blob/master/gif/ball.gif?raw=true)

OpenCat software works on both Nybble and Bittle, controlled by NyBoard based on ATmega328P. To run the code on our robot models, first change the model and board definition in **OpenCat.h**, then upload **WriteInstinct.ino**.


```
#include "InstinctBittle.h" //activate the correct header file according to your model
//#include "InstinctNybble.h"

//#define NyBoard_V0_1
//#define NyBoard_V0_2
#define NyBoard_V1_0
```

Set **No line ending** in the serial monitor and baudrate as **115200** (or **57600** for NyBoard V0_\*). Enter three capitalized **Y** after the prompts and wait for the MPU to calibrate. Then upload **OpenCat.ino** as the main functional code.

More detailed documentation can be found at the [Petoi Doc Center](https://docs.petoi.com).

You can use our mobile app to remote-control the robots as well:
* IOS: [App Store](https://apps.apple.com/us/app/petoi/id1581548095)
* Android: [Google Play](https://play.google.com/store/apps/details?id=com.petoi.petoiapp)

For updates:
* star this repository to receive timely notifications on changes.
* visit www.petoi.com and subscribe to our official newsletters for project announcements. We also host a forum at [petoi.camp](https://www.petoi.com/forum).
* follow us on [Twitter](https://twitter.com/petoicamp), [Instagram](https://www.instagram.com/petoicamp/), and [YouTube channel](https://www.youtube.com/c/rongzhongli) for fun videos and community activities.

![](https://github.com/PetoiCamp/NonCodeFiles/blob/master/gif/backflip.gif?raw=true)

[Advanced tutorials made by users](https://www.youtube.com/playlist?list=PLHMFXft_rV6MWNGyofDzRhpatxZuUZMdg)

[Review, open-box, and demos by users](https://www.youtube.com/playlist?list=PLHMFXft_rV6PSS3Qu5yQ-0iPW-ohu1sM3)

The [old repository for OpenCat](https://github.com/PetoiCamp/OpenCat-Old) is too redundant with large image logs and will be obsolete after adding compatibility notes in the documentation.
