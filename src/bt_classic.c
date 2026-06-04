#include "bt_classic.h"
#include "bt_keyboard.h"
#include "display.h"

#include <btstack.h>
#include <classic/hid_host.h>
#include <string.h>
#include <stdio.h>

#define CLASSIC_HID_DESCRIPTOR_SIZE 300
static uint8_t classic_hid_descriptor_storage[CLASSIC_HID_DESCRIPTOR_SIZE];
static uint16_t classic_hid_cid = 0;
static bool classic_connected = false;
static char classic_device_name[248] = "";
static bool classic_descriptor_available = false;
static hid_protocol_mode_t classic_protocol_mode = HID_PROTOCOL_MODE_REPORT_WITH_FALLBACK_TO_BOOT;

static void classic_hid_packet_handler(uint8_t packet_type, uint16_t channel,
                                        uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;

    uint8_t event = hci_event_packet_get_type(packet);
    if (event != HCI_EVENT_HID_META) return;

    uint8_t subevent = hci_event_hid_meta_get_subevent_code(packet);
    switch (subevent) {
        case HID_SUBEVENT_INCOMING_CONNECTION:
            printf("[CLASSIC] Incoming HID connection\n");
            hid_host_accept_connection(
                hid_subevent_incoming_connection_get_hid_cid(packet),
                classic_protocol_mode);
            break;

        case HID_SUBEVENT_CONNECTION_OPENED: {
            uint8_t status = hid_subevent_connection_opened_get_status(packet);
            if (status != ERROR_CODE_SUCCESS) {
                printf("[CLASSIC] Connection failed: 0x%02x\n", status);
                classic_hid_cid = 0;
                bt_keyboard_notify_classic_disconnected();
                return;
            }
            classic_hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
            classic_connected = true;
            classic_descriptor_available = false;
            printf("[CLASSIC] HID Host connected, cid=%u\n", classic_hid_cid);

            bd_addr_t addr;
            hid_subevent_connection_opened_get_bd_addr(packet, addr);
            if (classic_device_name[0] == '\0') {
                snprintf(classic_device_name, sizeof(classic_device_name),
                         "%s", bd_addr_to_str(addr));
            }
            bt_keyboard_notify_classic_connected(classic_device_name);
            break;
        }

        case HID_SUBEVENT_DESCRIPTOR_AVAILABLE: {
            uint8_t status = hid_subevent_descriptor_available_get_status(packet);
            if (status == ERROR_CODE_SUCCESS) {
                classic_descriptor_available = true;
                printf("[CLASSIC] HID descriptor available\n");
            } else {
                printf("[CLASSIC] No HID descriptor (boot mode), status=0x%02x\n", status);
            }
            break;
        }

        case HID_SUBEVENT_REPORT: {
            const uint8_t *report = hid_subevent_report_get_report(packet);
            uint16_t report_len = hid_subevent_report_get_report_len(packet);

            if (report_len < 2) return;
            if (report[0] != 0xa1) return;

            bt_keyboard_classic_report(report + 1, report_len - 1);
            break;
        }

        case HID_SUBEVENT_SET_PROTOCOL_RESPONSE: {
            uint8_t status = hid_subevent_set_protocol_response_get_handshake_status(packet);
            hid_protocol_mode_t mode = (hid_protocol_mode_t)hid_subevent_set_protocol_response_get_protocol_mode(packet);
            if (status == HID_HANDSHAKE_PARAM_TYPE_SUCCESSFUL) {
                printf("[CLASSIC] Protocol mode: %s\n",
                       mode == HID_PROTOCOL_MODE_BOOT ? "BOOT" : "REPORT");
            } else {
                printf("[CLASSIC] Set protocol failed: 0x%02x\n", status);
            }
            break;
        }

        case HID_SUBEVENT_CONNECTION_CLOSED:
            printf("[CLASSIC] HID disconnected\n");
            classic_hid_cid = 0;
            classic_connected = false;
            classic_descriptor_available = false;
            classic_device_name[0] = '\0';
            bt_keyboard_notify_classic_disconnected();
            break;

        case HID_SUBEVENT_SNIFF_SUBRATING_PARAMS:
            printf("[CLASSIC] Sniff subrating params: cid=%u max_latency=%u min_timeout=%u\n",
                   hid_subevent_sniff_subrating_params_get_hid_cid(packet),
                   hid_subevent_sniff_subrating_params_get_host_max_latency(packet),
                   hid_subevent_sniff_subrating_params_get_host_min_timeout(packet));
            break;

        default:
            printf("[CLASSIC] HID subevent 0x%02x\n", subevent);
            break;
    }
}

void bt_classic_init(void)
{
    hid_host_init(classic_hid_descriptor_storage, sizeof(classic_hid_descriptor_storage));
    hid_host_register_packet_handler(classic_hid_packet_handler);

    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_SNIFF_MODE | LM_LINK_POLICY_ENABLE_ROLE_SWITCH);
    hci_set_master_slave_policy(HCI_ROLE_MASTER);
    gap_discoverable_control(1);
    // Keep SSP enabled for modern keyboards; legacy PIN fallback still works.
    gap_ssp_set_enable(1);
    gap_ssp_set_io_capability(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    printf("[CLASSIC] HID host initialized\n");
}

void bt_classic_start_inquiry(void)
{
    printf("[CLASSIC] Starting inquiry (5s)\n");
    classic_device_name[0] = '\0';
    gap_inquiry_start(4);
}

void bt_classic_stop_inquiry(void)
{
    printf("[CLASSIC] Stopping inquiry\n");
    gap_inquiry_stop();
}

void bt_classic_connect(bd_addr_t addr)
{
    printf("[CLASSIC] Connecting to %s\n", bd_addr_to_str(addr));
    bt_keyboard_notify_classic_connecting(classic_device_name[0] ? classic_device_name : "Keyboard");
    uint8_t status = hid_host_connect(addr, classic_protocol_mode, &classic_hid_cid);
    if (status != ERROR_CODE_SUCCESS) {
        printf("[CLASSIC] hid_host_connect failed: 0x%02x\n", status);
        classic_hid_cid = 0;
    }
}

void bt_classic_disconnect(void)
{
    if (classic_hid_cid) {
        printf("[CLASSIC] Disconnecting\n");
        hid_host_disconnect(classic_hid_cid);
        classic_hid_cid = 0;
    }
    classic_connected = false;
}

bool bt_classic_is_connected(void)
{
    return classic_connected;
}

const char *bt_classic_get_device_name(void)
{
    return classic_device_name[0] ? classic_device_name : "Keyboard";
}

void bt_classic_set_device_name(const uint8_t *name, uint8_t name_len)
{
    if (name_len > sizeof(classic_device_name) - 1)
        name_len = sizeof(classic_device_name) - 1;
    memcpy(classic_device_name, name, name_len);
    classic_device_name[name_len] = '\0';
}
