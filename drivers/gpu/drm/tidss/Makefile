# SPDX-License-Identifier: GPL-2.0

tidss-y := tidss_crtc.o \
	tidss_drv.o \
	tidss_encoder.o \
	tidss_kms.o \
	tidss_irq.o \
	tidss_plane.o \
	tidss_scale_coefs.o \
	tidss_dispc.o

tidss-$(CONFIG_DRM_TIDSS_WB) += tidss_wb.o tidss_wb_m2m.o

obj-$(CONFIG_DRM_TIDSS) += tidss.o
