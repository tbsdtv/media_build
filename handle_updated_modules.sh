#!/bin/bash

# base for the final installation in "/lib/modules/..." (might be changed
# for different distributions)
# Note: DKMS handles most if the distribution specific part already!
mod_dest_loc_base="/kernel"

upd_mod_conf_name='dkms_updated_modules.conf'
mod_pattern='*.ko'
module_suffix='.ko'
install_tree="/lib/modules"


err_ok=0
err_usage=1
err_dir_not_found=2
err_tmp_failed=3
err_read_config=4
err_kernel_tree=5
err_action=6
err_default=20

modules_removed=""

readonly update_conf_variables="REMOVE_MODULE_NAME REMOVE_MODULE_LOCATION"

function exit_print {
    if [ -z "${2}" ] ; then
        code=${err_default}
    else
        code=${2}
    fi

    echo "${1}"
    exit ${code}
}

function err_exit {
    exit_print "Error: ${1}" ${2}
}

function usage {
    echo "Usage: $0 <dkms_base_dir> <kernelver> <arch> <action> [--help]"
    echo " <dkms_base_dir>: directory where the DKMS stores the build information"
    echo "                  (there are the 'module' and 'log' directories)."
    echo " <kernelver>: kernel version as used by DKMS"
    echo " <arch>: architecture as used by DKMS"
    echo " <action>: install or uninstall"
    echo " Options:"
    echo "   --help: This text"
    exit_print "" ${err_usage}
}

function arg_check_help {
    if [ "${1}" = "--help" ] ; then
        usage
    fi
}

mktemp_or_die() {
    local t
    t=$(mktemp "$@") && echo "$t" && return
    [[ $* = *-d* ]] && err_exit "Unable to make temporary directory" ${err_tmp_failed}
    err_exit "Unable to make temporary file." ${err_tmp_failed}
}

# copied from dkms shell script
function safe_source {
    # $1 = file to source
    # $@ = environment variables to echo out
    local to_source_file="$1"; shift
    declare -a -r export_envs=("$@")
    local tmpfile=$(mktemp_or_die)
    ( exec >"$tmpfile"
        . "$to_source_file" >/dev/null
        # This is really ugly, but a neat hack
        # Remember, in bash 2.0 and greater all variables are really arrays.
        for _export_env in "${export_envs[@]}"; do
            for _i in $(eval echo \${!$_export_env[@]}); do
            eval echo '$_export_env[$_i]=\"${'$_export_env'[$_i]}\"'
            done
        done
    )
    . "$tmpfile"
    rm "$tmpfile"
}

function read_config_file {
    local return_value=0

    # Clear variables and arrays
    for var in ${update_conf_variables}; do
        unset $var
    done

    # Source in the configuration file
    safe_source "${upd_mod_conf}" ${update_conf_variables}

    # Set module naming/location arrays
    local index array_size=0 s
    for s in ${#REMOVE_MODULE_NAME[@]} \
             ${#REMOVE_MODULE_LOCATION[@]}; do
        ((s > array_size)) && array_size=$s
    done
    for ((index=0; index < array_size; index++)); do
        # Set values
        remove_module_name[$index]=${REMOVE_MODULE_NAME[$index]}
        remove_module_location[$index]=${REMOVE_MODULE_LOCATION[$index]}

        # FAIL if no remove_module_name
        if [[ ! ${remove_module_name[$index]} ]]; then
            echo "$(basename ${upd_mod_conf}): Error! No 'REMOVE_MODULE_NAME' directive specified for record #$index."
            return_value=1
        fi
        if [[ ! ${REMOVE_MODULE_LOCATION[$index]} ]]; then
            echo "$(basename ${upd_mod_conf}): Error! No 'REMOVE_MODULE_LOCATION' directive specified for record #$index."
            return_value=1
        fi
    done

    # Fail if absolutely no REMOVE_MODULE_NAME
    if ((${#remove_module_name[@]} == 0)); then
        echo "upd_mod_conf: Error! No 'REMOVE_MODULE_NAME' directive specified."
        return_value=1
    fi

    # Fail if absolutely no REMOVE_MODULE_LOCATION
    if ((${#remove_module_location[@]} == 0)); then
        echo "upd_mod_conf: Error! No 'REMOVE_MODULE_LOCATION' directive specified."
        return_value=1
    fi

    return $return_value
}

function read_config_file_or_die {
    read_config_file "$@" && return

    err_exit "Bad conf file (${1})." ${err_read_config}
}

function remove_modules {
    local kernel_tree="${install_tree}/${kernelver}"
    local dkms_original="${dkms_base_dir}/original_module"

    [[ -e ${kernel_tree} ]] || err_exit "Kernel tree ${kernel_tree} doesn't exist." ${err_kernel_tree}

    for ((count=0; count < ${#remove_module_name[@]}; count++)); do
        local kernel_mod=${kernel_tree}${remove_module_location[$count]}/${remove_module_name[$count]}${module_suffix}
        local dkms_orig=${dkms_original}/${remove_module_name[$count]}${module_suffix}

        echo ""
        echo "${remove_module_name[$count]}${module_suffix}:"
        if [ -e ${dkms_orig} ]; then
            echo " - An original module was already stored during a previous install"
        else
            if [ -f "${kernel_mod}" ]; then
                echo " - Found ${kernel_mod}"
                echo " - Storing in ${dkms_original}/"
                echo " - Archiving for uninstallation purposes"
                mkdir -p "${dkms_original}/"
                mv -f "${kernel_mod}" "${dkms_orig}"
                modules_removed="true"
            fi
        fi
    done

    echo ""
}

function restore_modules {
    local kernel_tree="${install_tree}/${kernelver}"
    local dkms_original="${dkms_base_dir}/original_module"
    local moved=""

    [[ -e ${kernel_tree} ]] || err_exit "Kernel tree ${kernel_tree} doesn't exist." ${err_kernel_tree}

    for ((count=0; count < ${#remove_module_name[@]}; count++)); do
        local kernel_mod=${kernel_tree}${remove_module_location[$count]}/${remove_module_name[$count]}${module_suffix}
        local dkms_orig=${dkms_original}/${remove_module_name[$count]}${module_suffix}

        echo ""
        echo "${remove_module_name[$count]}${module_suffix}:"
        if [ -e ${dkms_orig} ]; then
            local kernel_mod_dir=$(dirname ${kernel_mod})
		    echo " - Archived original module found in the DKMS tree"
		    echo " - Moving it to: ${kernel_mod_dir}/"
		    mkdir -p "${kernel_mod_dir}/"
            mv -f "${dkms_orig}" "${kernel_mod_dir}/" 2> /dev/null
            moved="true"
        else 
            echo " - No original module was found for this module on this kernel."
            echo " - Use the dkms install command to reinstall any previous module version."
        fi
    done

    if [ ${moved} ]; then
        [[ $(find ${dkms_original}/* -maxdepth 0 -type f 2>/dev/null) ]] || rm -rf "${dkms_original}"
    fi

    echo ""
}

# main

if [ $# -lt 4 ] ; then
    usage
fi

arg_check_help "${1}"

dkms_base_dir="${1}"
shift

arg_check_help "${1}"

kernelver="${1}"
shift

arg_check_help "${1}"

arch="${1}"
shift

arg_check_help "${1}"

action="${1}"
shift

arg_check_help "${1}"

# First check, if the DKMS base directory, does already exist
if [ ! -d ${dkms_base_dir} ] ; then
    err_exit "DKMS base directory ${dkms_base_dir} doesn't exist!" ${err_dir_not_found}
fi

script_dir=$(dirname ${BASH_SOURCE[0]})
upd_mod_conf=${script_dir}/${upd_mod_conf_name}

# Check for the configuration file
if [ -f ${upd_mod_conf} ] ; then
    echo "Using configuration file ${upd_mod_conf}"

    read_config_file_or_die ${upd_mod_conf} 

    case "${action}" in
        install) remove_modules
                 if [ ${modules_removed} ]; then
                     echo "!!!! NOTE NOTE NOTE  !!!!"
                     echo " There is NO uninstall hook in DKMS available."
                     echo " This script has saved some modules to"
                     echo "   ${dkms_base_dir}/original_module"
                     echo " Prior to uninstalling this DKMS module execute"
                     echo "   $0 ${dkms_base_dir} ${kernelver} ${arch} uninstall"
                     echo ""
                 fi
                 ;;
        uninstall) restore_modules ;;
        *) err_exit "Invalid action '${action}' given!" ${err_action}
    esac


    exit_print "Done!" ${err_ok}
else
    exit_print "Nothing to do!" ${err_ok}
fi

