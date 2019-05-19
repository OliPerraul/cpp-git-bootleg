* BOOST
https://www.boost.org/users/download/
How to install boost
https://www.ibm.com/support/knowledgecenter/en/SSWTQQ_1.0.0/com.ibm.swg.ba.cognos.trade_analytics.1.0.0.doc/t_trd_installboost.html

* Set this environment variables:

BOOST_ROOT
C:\boost_1_70_0

* CMake in visual studio tutorial (Visual Studio 2017)
https://www.youtube.com/watch?v=_lKxJjV8r3Y


* See program_option for an example of how I add a dependency


Il ne suffit que de s'inspirer de ce fichier
https://github.com/benhoyt/pygit/blob/aa8d8bb62ae273ae2f4f167e36f24f40a11634b9/pygit.py



//.git/index
https://mincong-h.github.io/2018/04/28/git-index/


// info qu'on devrait stocker a la place (mais on met zero)
https://en.wikipedia.org/wiki/Stat_(system_call)


// Pygit
https://github.com/benhoyt/pygit/blob/master/pygit.py


// Pygit guide
https://benhoyt.com/writings/pygit/

----

//Zlib instruction

* 1. dowload zlib from here
http://zlib.net/

* 2. run b2 with the following command in powershell
./b2 -j15 --toolset=msvc --build-type=complete stage --with-iostreams -s ZLIB_BINARY=zlib -s ZLIB_SOURCE="C:\zlib-1.2.11\" -s ZLIB_INCLUDE="C:\zlib-1.2.11\"
https://stackoverflow.com/questions/23107703/compiling-boost-with-zlib-on-windows


NOTE:
*If didnt work try to build zlib before step 2.*

* 1.1 Build zlib
https://www.youtube.com/watch?time_continue=17&v=zco5runaFWU


----

**TODO**

* Commit 'nothing to commit working tree clean'
* Commit Test
* gitlab setup





