#!/bin/bash
make unapply-patches
make distclean
zip -r linux-media-src.zip backports linux v4l Makefile  INSTALL COPYING install.sh -x *.linked_dir -x *.gitignore -x *git_log
exit
