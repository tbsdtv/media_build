#!/bin/bash
make cleanall
make dir DIR=./media
zip -r linux-media-src.zip backports linux v4l Makefile  INSTALL COPYING install.sh -x *.linked_dir -x *.gitignore -x *git_log
exit
