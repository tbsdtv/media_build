# With VDR this gives tons of loggings
disable_opt CONFIG_DVB_DEMUX_SECTION_LOSS_LOG

# builds fine with this drivers, but keep them for later
#disable_opt CONFIG_VIDEOBUF2_MEMOPS

# CONFIG_FRAME_VECTOR causes troubles on some systems/distros
disable_opt CONFIG_FRAME_VECTOR

# MSI is currently still not reliable on Digital Devices cards
disable_opt CONFIG_DVB_DDBRIDGE_MSIENABLE

# There are systems with more high end Digital Devices cards with lots
# of tuners
set_opt_value CONFIG_DVB_MAX_ADAPTERS 64
