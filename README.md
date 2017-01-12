# MTXOS
A very basic Linux kernel.

This is a basic Linux kernel written for a class at WSU. The class used the book "Design and Implementation of 
the MTX Operating System" by Kung-Chi Wang, who is also the instructor for the class. 

With much guidance and weeks of torture, this mostly-functional OS kernel was the result. For almost every else 
in the class, at least, this was torture. I truly enjoyed it. The feeling of knowing that you're one step away 
from directly tickling the CPU was fantastic. Realizing that your OS runs on your computer but doesn't work on 
anyone else's because of hardware differences was incredible. My one regret is that I didn't opt to continue 
the series with the graduate-level OS class.

But all of that aside, compiling and running this kernel requires several things. First, a Linux machine. Second, 
Bruce's C Compiler, which can be installed using "sudo apt-get install bcc". Third, Qemu, which can be installed 
with "sudo apt-get install qemu". Lastly, the code itself. 

First, go into the user directory and run the "mku" script. This step requires sudo in order to mount the virtual 
disk and copy the user-mode shell for use. Go back up to the main directory and run the "compile" script. Open 
at least two more terminals (tabs work fine too) and follow the instruction in the "run" script. Then 
run the "run" script. This script also requires sudo in order to mount the disk. Hit enter at the prompt and Qemu 
should start the OS. 

At this point, I can only hope that it actually works for you. Considering how low-level this is and the fact that I 
tested it on a single machine with an Intel processor, I can't guarantee it'll work on anything else. Also, I think 
it'll work better on 32-bit machines. If the main Qemu terminal seems to be unresponsive, it's likely because you're on 
a 64-bit OS. The other two terminals should still be usable, though.

Known bugs:

Any characters that should appear in the lower right corner get obscured by the time. They are then lost. 

Unknown bugs:

As many as there are stars in the sky. Probably. It could be more. I'm trying to be optimistic here.
