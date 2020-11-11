/* Copyright (c) 2016-2020 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#ifndef EUI_H
#define EUI_H

/**
 * @file electricui.h
 * @brief You want to import and use this header.
 *
 * Provides application layer API surface to interact with Electric UI interfaces.
 * Supports tracking variables to minimise developer effort, and offers helpers for creating and sending packets.
 *
 * @see https://electricui.com/docs/quick-start/overview for architectural overview
 * */

#ifdef __cplusplus
extern "C" {
#endif

#include "eui_config.h"
#include "eui_macro.h"
#include "eui_types.h"

// Warnings based on configuration flags (depends on compiler support)
#ifdef EUI_CONF_OFFSETS_DISABLED
    #warning "ElectricUI will not handle data larger than PAYLOAD_SIZE_MAX"
#endif

#define EUI_LIBRARY_VERSION 8u

/**
 * @brief Interfaces contain local storage for inbound packets, and pointers for data output callbacks, and state callbacks
 *
 * Each eui_interface_t instance represents a communications link such as a specific serial connection.
 * When inbound data is read by the library, this data structure provides the persistent storage of the inbound parser state/buffers.
 * If the library needs to generate a response, it uses the output_cb function pointer.
 * When the library succeeds or fails in parsing a packet, the optional interface_cb provides user-space callbacks to support power-users.
 *
 * Providing an array of communications interfaces allows Electric UI to operate over many different links at the same time, and the UI can
 * use this functionality to power multi-interface connections with failover and load-balancing.
 *
 * @see EUI_INTERFACE and EUI_INTERFACE_CB for syntactic sugar
 */
typedef struct {
    eui_packet_t        packet;         ///< Stores the parser statemachine data and intermediate buffers as data is parsed byte-by-byte
    callback_data_out_t output_cb;      ///< Points to the user-space data egress function for this interface (where you write to UART, etc)
    callback_uint8_t    interface_cb;   ///< Points to a user-space callback, includes eui_callback_codes flag describing callback generation context
} eui_interface_t;

/**
 * @brief Get a pointer to the tracked object by name
 *
 * Searches through the user-space tracked variable array (if provided), and returns a pointer to the first exact matching identifier string.
 *
 * @param search_id Identifier string to search for
 * @return Pointer to a tracked variable in the array of user-space tracked variables. Further destructuring can provide type information or the payload pointer.
 * @return Returns 0 if no match is found.
 */
eui_message_t *
find_tracked_object( const char * search_id );

/**
 * @brief Provides data ingest for inbound data, packet handling
 *
 * This function should be called for every byte that is received on a communications link. As valid data is received, the parser will reconstruct the packet structure and
 * when valid data is ingested, payload handling will occur.
 *
 * Provides most of the functionality for this library, will generate response messages or mutate tracked variables if new data is written to the device.
 *
 * Do not invoke this function from an interrupt, as the handler is likely to generate output data and other callbacks.
 *
 * @param inbound_byte A single byte of data (uint8_t) to parse
 * @param p_link Pointer to the interface from which data was recieved. This structure provides persistent storage and output callbacks.
 * @return eui_errors_t provides detailed sub-system error/success state information
 */
eui_errors_t
eui_parse( uint8_t inbound_byte, eui_interface_t *p_link );

/**
 * @brief Low level message output
 *
 * Allows for control over the output callback writer, along with packet settings for custom header requirements.
 * This is mostly used internally to generate messages from known interfaces and found/provided tracked variable pointers.
 * It can be used to manually form less typical messages such as requests against the UI, type overrides or generate impostor internal messages etc.
 *
 * @param output_function Pointer to the user-space output function where data will be written
 * @param p_msg_obj Pointer to a eui_message_t
 * @param settings Shorthand packet header settings
 * @return eui_output_errors provides EUI_OUTPUT_OK or EUI_OUTPUT_ERROR
 * @see eui_send_range which is used if the data length exceeds a single packet
 */
uint8_t
eui_send(   callback_data_out_t output_function,
            eui_message_t       *p_msg_obj,
            eui_pkt_settings_t  *settings );

#ifndef EUI_CONF_OFFSETS_DISABLED
/**
 * @brief Send a slice of a payload as an offset message
 *
 * Offset messages are used for data transfer larger than one packet, and can be used for partial updates to a section of data.
 * By providing the start and end offset sizes, just the relevant data is sent.
 * The UI _should_ be able to perform a delta-update, or combine multiple slices.
 *
 * This function is mostly intended for internal use, as eui_send_* functions call this if required.
 *
 * @param output_function  Pointer to the user-space output function where data will be written
 * @param p_msg_obj Pointer to a eui_message_t
 * @param settings Shorthand packet header settings
 * @param base_addr Index/Start offset of the first byte in the slice
 * @param end_addr Index/End offset of the last byte in the slice
 * @return eui_output_errors provides EUI_OUTPUT_OK or EUI_OUTPUT_ERROR
 */
    uint8_t
    eui_send_range( callback_data_out_t output_function,
                    eui_message_t       *p_msg_obj,
                    eui_pkt_settings_t  *settings,
                    uint16_t            base_addr,
                    uint16_t            end_addr );
#endif

/**
 * Provide a interface to the library for use
 * Calls eui_setup_interfaces() with link_count of 1
 * @param link Pointer to a eui_interface_t instantiated in your code
 */
void
eui_setup_interface( eui_interface_t *link );

/** Provide interface(s) to the library for use
 * \param link_array
 * \param link_count
 */
void
eui_setup_interfaces( eui_interface_t *link_array, uint8_t link_count );

/**
 * @brief Configure tracked variable management
 *
 * Tracked variables are developer provided eui_message_t which provide a set of identifiers and pointers to developer-scoped data
 * which the library is allowed to mutate. This means the library will automatically process inbound requests or writes against data,
 * and includes automatic inclusion of these variables into the handshake process.
 * @param msg_array Pointer to a developer owned array of eui_message_t
 * @param num_tracked Number of tracked variables in the array
 * @see EUI_TRACK for syntactic sugar
 */
void
eui_setup_tracked( eui_message_t *msg_array, eui_variable_count_t num_tracked );

/**
 * @brief Provide a unique identifier to distinguish between hardware boards running the same firmware
 *
 * Generates the UUID by running the user-provided data through CRC16
 * This allows the user interface to detect when a particular device is accessible over multiple communication interfaces,
 * and to differentiate otherwise identical devices connected to the same computer.
 * Seed this with ideally deterministic and unique data like a chip ID, mac address or serial number
 *
 * @param uuid Unique data/text specific to this device
 * @param bytes Size of seed data
 */
void
eui_setup_identifier( char * uuid, uint8_t bytes );

/**
 * @brief Send the tracked message which matches the input message identifier
 *
 * Selects the output communications interface automatically (typically the last active interface)
 * @param msg_id
 * @see eui_send_tracked_on
 */
void
eui_send_tracked( const char * msg_id );

/**
 * @brief Send the tracked message matching the input message identifier over a specific interface
 * @param msg_id
 * @param interface
 */
void
eui_send_tracked_on( const char * msg_id, eui_interface_t *interface );

/**
 * @brief Send an untracked message
 * @param p_msg_obj
 * @see eui_send_untracked_on
 */
void
eui_send_untracked( eui_message_t *p_msg_obj );

/**
 * @brief Send an untracked message over a specific interface
 * @param p_msg_obj
 * @param interface
 */
void
eui_send_untracked_on( eui_message_t *p_msg_obj, eui_interface_t *interface );

#ifdef __cplusplus
}
#endif

#endif //end EUI_H