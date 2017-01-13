#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

########################
LOCAL_PATH := hardware/realtek/wlan/rtl/firmware

-include hardware/realtek/wlan/rtl/config/config-rtl.mk

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rtl8703b_config:system/etc/firmware/rtl8703bs_config \
    $(LOCAL_PATH)/rtl8703b_fw:system/etc/firmware/rtl8703bs_fw \
    $(LOCAL_PATH)/rtl8723a_config:system/etc/firmware/rtl8723as_config \
    $(LOCAL_PATH)/rtl8723a_fw:system/etc/firmware/rtl8723as_fw \
    $(LOCAL_PATH)/rtl8723b_config:system/etc/firmware/rtl8723bs_config \
    $(LOCAL_PATH)/rtl8723b_fw:system/etc/firmware/rtl8723bs_fw \
    $(LOCAL_PATH)/rtl8723b_VQ0_config:system/etc/firmware/rtl8723bs_VQ0_config \
    $(LOCAL_PATH)/rtl8723b_VQ0_fw:system/etc/firmware/rtl8723bs_VQ0_fw \
    $(LOCAL_PATH)/rtl8723cs_cg_config:system/etc/firmware/rtl8723cs_cg_config \
    $(LOCAL_PATH)/rtl8723cs_cg_fw:system/etc/firmware/rtl8723cs_cg_fw \
    $(LOCAL_PATH)/rtl8723cs_vf_config:system/etc/firmware/rtl8723cs_vf_config \
    $(LOCAL_PATH)/rtl8723cs_vf_fw:system/etc/firmware/rtl8723cs_vf_fw \
    $(LOCAL_PATH)/rtl8723cs_xx_config:system/etc/firmware/rtl8723cs_xx_config \
    $(LOCAL_PATH)/rtl8723cs_xx_fw:system/etc/firmware/rtl8723cs_xx_fw

