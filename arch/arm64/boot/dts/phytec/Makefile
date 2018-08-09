# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2018-2019 PHYTEC America, LLC - https://www.phytec.com
#

DTC_FLAGS += -@

dtb-$(CONFIG_ARCH_K3_AM6_SOC) += k3-am65xx-phycore-kit.dtb

$(obj)/%.dtbo: $(src)/%.dtso FORCE
	$(call if_changed_dep,dtc)

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
