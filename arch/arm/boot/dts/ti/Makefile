DTC_FLAGS += -@

# base dtbs with symbols enabled
dtb-y += \
	am571x-idk.dtb \
	am572x-idk.dtb \
	am574x-idk.dtb \
	am57xx-beagle-x15.dtb \
	am57xx-beagle-x15-revb1.dtb \
	am57xx-beagle-x15-revc.dtb \
	dra71-evm.dtb \
	dra72-evm.dtb \
	dra72-evm-revc.dtb \
	dra76-evm.dtb \
	dra7-evm.dtb

# overlays
dtb-y += am57xx-evm.dtbo \
	am57xx-evm-common.dtbo \
	am57xx-evm-reva3.dtbo \
	ov10635.dtbo \
	lcd-osd101t2045.dtbo lcd-osd101t2587.dtbo \
	dra72-evm-touchscreen.dtbo dra74-evm-touchscreen.dtbo \
	dra7x-evm-osd-lcd-common.dtbo \
	am571x-idk-touchscreen.dtbo am572x-idk-touchscreen.dtbo \
	am57xx-idk-osd-lcd-common.dtbo \
	dra71-evm-lcd-auo-g101evn01.0.dtbo \
	dra71-evm-nand.dtbo \
	dra76-evm-tfp410.dtbo \
	am57xx-idk-pps.dtbo

ifneq ($(DTC),)
  DTC_PATH=$(dir $(DTC))
else
  DTC_PATH=$(objtree)/scripts/dtc
endif

$(obj)/%.dtb: $(src)/../%.dts FORCE
	$(call if_changed_dep,dtc,dtb)

$(obj)/%.dtbo: $(src)/%.dtso FORCE
	$(call if_changed_dep,dtc,dtb)

clean-files	:= *.dtb *.dtbo

always		:= $(dtb-y)
