<pre>
   ##    #    #   ####    ####   #    #
  #  #   ##  ##  #    #  #       #    #
 #    #  # ## #  #        ####   ######
 ######  #    #  #            #  #    #
 #    #  #    #  #    #  #    #  #    #
 #    #  #    #   ####    ####   #    #
</pre>

[ amcsh ] - A More Comfortable SHell

by t57root @ openwill.me 

&lt;t57root@gmail.com>  [www.HackShell.net](http://www.hackshell.net/)


###Features:

* Full pty support: VIM, SSH, readline
* Authentication support: pre-shared password / one-time password(based on google authenticator)
* Reverse connection / Bind port 
* Fake argv: Fake CMD in ps,netstat

###Help Message:

* Configure:

>>All configurable variables can be found in amcsh.h.

>>Check HOWTO.md for more details.

* Compile:

>>Run "make" to compile amcsh and obtain executables.

* Usage:

>>./client {listen|connect} ip port

>>[listen]: Listen at ip:port for connection as a server (Reverse connection mode)

>>[connect]: Connect to ip:port as a client (Bind port mode)

