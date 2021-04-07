#!/bin/bash

# get DKMS version to variable PACKAGE_VERSION
. dkms_ver.conf

DKMS_TAR_NAME=media-build-${PACKAGE_VERSION}.dkms_src.tgz

tar_dirs="backports "
tar_dirs+="devel_scripts "
tar_dirs+="linux "
tar_dirs+="v4l "

tar_files="*.inc "
tar_files+="build_all.sh "
tar_files+="COPYING "
tar_files+="dkms.conf "
tar_files+="dkms_ver.conf "
tar_files+="gen_dkms_dyn_conf.sh "
tar_files+="INSTALL "
tar_files+="Makefile "
tar_files+="Makefile.dkms "
tar_files+="README_dkms "
tar_files+="handle_updated_modules.sh "
tar_files+="dkms_updated_modules.conf "

rm -f ${DKMS_TAR_NAME}
tar -czf ${DKMS_TAR_NAME} ${tar_dirs} ${tar_files}

