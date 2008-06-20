/* $Id$ */
/* 
 * Copyright (C) 2003-2007 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#ifndef __SIP_INVITE_SESSION_H__
#define __SIP_INVITE_SESSION_H__

/**
 * @file sip_inv.h
 * @brief INVITE sessions
 */


#include <pjsip/sip_dialog.h>
#include <pjmedia/sdp_neg.h>


/**
 * @defgroup PJSIP_HIGH_UA User Agent Library
 * @brief Mid-level User Agent Library.
 *
 * This is the high level user agent library, which consists of:
 *  - @ref PJSIP_INV, to encapsulate INVITE sessions and SDP
 *    negotiation in the session,
 *  - @ref PJSUA_REGC, high level client registration API, and
 *  - @ref PJSUA_XFER.
 *
 * More detailed information is explained in
 * <A HREF="/docs.htm">PJSIP Developer's Guide</A>
 * PDF document, and readers are encouraged to read the document to
 * get the concept behind dialog, dialog usages, and INVITE sessions.
 *
 * The User Agent Library is implemented in <b>pjsip-ua</b> static
 * library.
 */

/**
 * @defgroup PJSIP_INV INVITE Session
 * @ingroup PJSIP_HIGH_UA
 * @brief Provides INVITE session management.
 * @{
 *
 * The INVITE session uses the @ref PJSIP_DIALOG framework to manage
 * the underlying dialog, and is one type of usages that can use
 * a particular dialog instance (other usages are event subscription,
 * discussed in @ref PJSIP_EVENT_NOT). The INVITE session manages
 * the life-time of the session, and also manages the SDP negotiation.
 *
 * Application must link with  <b>pjsip-ua</b> static library to use this API.
 *
 * More detailed information is explained in
 * <A HREF="/docs.htm">PJSIP Developer's Guide</A>
 * PDF document, and readers are encouraged to read the document to
 * get the concept behind dialog, dialog usages, and INVITE sessions.
 *
 * The INVITE session does NOT manage media. If application wants to
 * use API that encapsulates both signaling and media in a very easy
 * to use API, it can use @ref PJSUA_LIB for this purpose.
 */

PJ_BEGIN_DECL


/**
 * @see pjsip_inv_session
 */
typedef struct pjsip_inv_session pjsip_inv_session;


/**
 * This enumeration describes invite session state.
 */
typedef enum pjsip_inv_state
{
    PJSIP_INV_STATE_NULL,	    /**< Before INVITE is sent or received  */
    PJSIP_INV_STATE_CALLING,	    /**< After INVITE is sent		    */
    PJSIP_INV_STATE_INCOMING,	    /**< After INVITE is received.	    */
    PJSIP_INV_STATE_EARLY,	    /**< After response with To tag.	    */
    PJSIP_INV_STATE_CONNECTING,	    /**< After 2xx is sent/received.	    */
    PJSIP_INV_STATE_CONFIRMED,	    /**< After ACK is sent/received.	    */
    PJSIP_INV_STATE_DISCONNECTED,   /**< Session is terminated.		    */
} pjsip_inv_state;

/**
 * This structure contains callbacks to be registered by application to 
 * receieve notifications from the framework about various events in
 * the invite session.
 */
typedef struct pjsip_inv_callback
{
    /**
     * This callback is called when the invite sesion state has changed. 
     * Application should inspect the session state (inv_sess->state) to get 
     * the current state of the session.
     *
     * This callback is mandatory.
     *
     * @param inv	The invite session.
     * @param e		The event which has caused the invite session's 
     *			state to change.
     */
    void (*on_state_changed)(pjsip_inv_session *inv, pjsip_event *e);


    /**
     * This callback is called when the invite usage module has created 
     * a new dialog and invite because of forked outgoing request.
     *
     * This callback is mandatory.
     *
     * @param inv	The new invite session.
     * @param e		The event which has caused the dialog to fork.
     *			The type of this event can be either
     *			PJSIP_EVENT_RX_MSG or PJSIP_EVENT_RX_200_MSG.
     */
    void (*on_new_session)(pjsip_inv_session *inv, pjsip_event *e);

    /**
     * This callback is called whenever any transactions within the session
     * has changed their state. Application MAY implement this callback, 
     * e.g. to monitor the progress of an outgoing request, or to send
     * response to unhandled incoming request (such as INFO).
     *
     * This callback is optional.
     *
     * @param inv	The invite session.
     * @param tsx	The transaction, which state has changed.
     * @param e		The event which has caused the transation state's
     *			to change.
     */
    void (*on_tsx_state_changed)(pjsip_inv_session *inv,
				 pjsip_transaction *tsx,
				 pjsip_event *e);

    /**
     * This callback is called when the invite session has received 
     * new offer from peer. Application can inspect the remote offer 
     * in "offer", and set the SDP answer with #pjsip_inv_set_sdp_answer().
     * When the application sends a SIP message to send the answer, 
     * this SDP answer will be negotiated with the offer, and the result
     * will be sent with the SIP message.
     *
     * @param inv	The invite session.
     * @param offer	Remote offer.
     */
    void (*on_rx_offer)(pjsip_inv_session *inv,
			const pjmedia_sdp_session *offer);

    /**
     * This callback is optional, and it is used to ask the application
     * to create a fresh offer, when the invite session has received 
     * re-INVITE without offer. This offer then will be sent in the
     * 200/OK response to the re-INVITE request.
     *
     * If application doesn't implement this callback, the invite session
     * will send the currently active SDP as the offer.
     *
     * @param inv	The invite session.
     * @param p_offer	Pointer to receive the SDP offer created by
     *			application.
     */
    void (*on_create_offer)(pjsip_inv_session *inv,
			    pjmedia_sdp_session **p_offer);

    /**
     * This callback is called after SDP offer/answer session has completed.
     * The status argument specifies the status of the offer/answer, 
     * as returned by pjmedia_sdp_neg_negotiate().
     * 
     * This callback is optional (from the point of view of the framework), 
     * but all useful applications normally need to implement this callback.
     *
     * @param inv	The invite session.
     * @param status	The negotiation status.
     */
    void (*on_media_update)(pjsip_inv_session *inv_ses, 
			    pj_status_t status);

    /**
     * This callback is called when the framework needs to send
     * ACK request after it receives incoming  2xx response for 
     * INVITE. It allows application to manually handle the 
     * transmission of ACK request, which is required by some 3PCC
     * scenarios. If this callback is not implemented, the framework
     * will handle the ACK transmission automatically.
     *
     * When this callback is overridden, application may delay the
     * sending of the ACK request (for example, when it needs to 
     * wait for answer from the other call leg, in 3PCC scenarios). 
     *
     * Application creates the ACK request
     *
     * Once it has sent the ACK request, the framework will keep 
     * this ACK request in the cache. Subsequent receipt of 2xx response
     * will not cause this callback to be called, and instead automatic
     * retransmission of this ACK request from the cache will be done
     * by the framework.
     *
     * This callback is optional.
     */
    void (*on_send_ack)(pjsip_inv_session *inv, pjsip_rx_data *rdata);

} pjsip_inv_callback;



/**
 * This enumeration shows various options that can be applied to a session. 
 * The bitmask combination of these options need to be specified when 
 * creating a session. After the dialog is established (including early), 
 * the options member of #pjsip_inv_session shows which capabilities are 
 * common in both endpoints.
 */
enum pjsip_inv_option
{	
    /** 
     * Indicate support for reliable provisional response extension 
     */
    PJSIP_INV_SUPPORT_100REL	= 1,

    /** 
     * Indicate support for session timer extension. 
     */
    PJSIP_INV_SUPPORT_TIMER	= 2,

    /** 
     * Indicate support for UPDATE method. This is automatically implied
     * when creating outgoing dialog. After the dialog is established,
     * the options member of #pjsip_inv_session shows whether peer supports
     * this method as well.
     */
    PJSIP_INV_SUPPORT_UPDATE	= 4,

    /** 
     * Require reliable provisional response extension. 
     */
    PJSIP_INV_REQUIRE_100REL	= 32,

    /**  
     * Require session timer extension. 
     */
    PJSIP_INV_REQUIRE_TIMER	= 64,

};


/**
 * This structure describes the invite session.
 */
struct pjsip_inv_session
{
    char		 obj_name[PJ_MAX_OBJ_NAME]; /**< Log identification */
    pj_pool_t		*pool;			    /**< Dialog's pool.	    */
    pjsip_inv_state	 state;			    /**< Invite sess state. */
    pj_bool_t		 cancelling;		    /**< CANCEL requested   */
    pj_bool_t		 pending_cancel;	    /**< Wait to send CANCEL*/
    pjsip_status_code	 cause;			    /**< Disconnect cause.  */
    pj_str_t		 cause_text;		    /**< Cause text.	    */
    pj_bool_t		 notify;		    /**< Internal.	    */
    pjsip_dialog	*dlg;			    /**< Underlying dialog. */
    pjsip_role_e	 role;			    /**< Invite role.	    */
    unsigned		 options;		    /**< Options in use.    */
    pjmedia_sdp_neg	*neg;			    /**< Negotiator.	    */
    pjsip_transaction	*invite_tsx;		    /**< 1st invite tsx.    */
    pjsip_tx_data	*last_answer;		    /**< Last INVITE resp.  */
    pjsip_tx_data	*last_ack;		    /**< Last ACK request   */
    pj_int32_t		 last_ack_cseq;		    /**< CSeq of last ACK   */
    void		*mod_data[PJSIP_MAX_MODULE];/**< Modules data.	    */
};


/**
 * Initialize the invite usage module and register it to the endpoint. 
 * The callback argument contains pointer to functions to be called on 
 * occurences of events in invite sessions.
 *
 * @param endpt		The endpoint instance.
 * @param cb		Callback structure.
 *
 * @return		PJ_SUCCESS on success, or the appropriate error code.
 */
PJ_DECL(pj_status_t) pjsip_inv_usage_init(pjsip_endpoint *endpt,
					  const pjsip_inv_callback *cb);

/**
 * Get the INVITE usage module instance.
 *
 * @return		PJ_SUCCESS on success, or the appropriate error code.
 */
PJ_DECL(pjsip_module*) pjsip_inv_usage_instance(void);


/**
 * Dump user agent contents (e.g. all dialogs).
 */
PJ_DECL(void) pjsip_inv_usage_dump(void);


/**
 * Create UAC invite session for the specified dialog in dlg. 
 *
 * @param dlg		The dialog which will be used by this invite session.
 * @param local_sdp	If application has determined its media capability, 
 *			it can specify the SDP here. Otherwise it can leave 
 *			this to NULL, to let remote UAS specifies an offer.
 * @param options	The options argument is bitmask combination of SIP 
 *			features in pjsip_inv_options enumeration.
 * @param p_inv		On successful return, the invite session will be put 
 *			in this argument.
 *
 * @return		The function will return PJ_SUCCESS if it can create
 *			the session. Otherwise the appropriate error status 
 *			will be returned on failure.
 */
PJ_DECL(pj_status_t) pjsip_inv_create_uac(pjsip_dialog *dlg,
					  const pjmedia_sdp_session *local_sdp,
					  unsigned options,
					  pjsip_inv_session **p_inv);


/**
 * Application SHOULD call this function upon receiving the initial INVITE 
 * request in rdata before creating the invite session (or even dialog), 
 * to verify that the invite session can handle the INVITE request. 
 * This function verifies that local endpoint is capable to handle required 
 * SIP extensions in the request (i.e. Require header) and also the media, 
 * if media description is present in the request.
 *
 * @param rdata		The incoming INVITE request.
 *
 * @param options	Upon calling this function, the options argument 
 *			MUST contain the desired SIP extensions to be 
 *			applied to the session. Upon return, this argument 
 *			will contain the SIP extension that will be applied 
 *			to the session, after considering the Supported, 
 *			Require, and Allow headers in the request.
 *
 * @param sdp		If local media capability has been determined, 
 *			and if application wishes to verify that it can 
 *			handle the media offer in the incoming INVITE 
 *			request, it SHOULD specify its local media capability
 *			in this argument. 
 *			If it is not specified, media verification will not
 *			be performed by this function.
 *
 * @param dlg		If tdata is not NULL, application needs to specify
 *			how to create the response. Either dlg or endpt
 *			argument MUST be specified, with dlg argument takes
 *			precedence when both are specified.
 *
 *			If a dialog has been created prior to calling this 
 *			function, then it MUST be specified in dlg argument. 
 *			Otherwise application MUST specify the endpt argument
 *			(this is useful e.g. when application wants to send 
 *			the response statelessly).
 *
 * @param endpt		If tdata is not NULL, application needs to specify
 *			how to create the response. Either dlg or endpt
 *			argument MUST be specified, with dlg argument takes
 *			precedence when both are specified.
 *
 * @param tdata		If this argument is not NULL, this function will 
 *			create the appropriate non-2xx final response message
 *			when the verification fails.
 *
 * @return		If everything has been negotiated successfully, 
 *			the function will return PJ_SUCCESS. Otherwise it 
 *			will return the reason of the failure as the return
 *			code.
 *
 *			This function is capable to create the appropriate 
 *			response message when the verification has failed. 
 *			If tdata is specified, then a non-2xx final response
 *			will be created and put in this argument upon return,
 *			when the verification has failed. 
 *
 *			If a dialog has been created prior to calling this 
 *			function, then it MUST be specified in dlg argument. 
 *			Otherwise application MUST specify the endpt argument
 *			(this is useful e.g. when application wants to send 
 *			the response statelessly).
 *
 * @see pjsip_inv_verify_request2()
 */
PJ_DECL(pj_status_t) pjsip_inv_verify_request(	pjsip_rx_data *rdata,
						unsigned *options,
						const pjmedia_sdp_session *sdp,
						pjsip_dialog *dlg,
						pjsip_endpoint *endpt,
						pjsip_tx_data **tdata);

/**
 * Variant of #pjsip_inv_verify_request() which allows application to specify
 * the parsed SDP in the \a offer argument. This is useful to avoid having to
 * re-parse the SDP in the incoming request.
 *
 * @see pjsip_inv_verify_request()
 */
PJ_DECL(pj_status_t) pjsip_inv_verify_request2( pjsip_rx_data *rdata,
						unsigned *options,
						const pjmedia_sdp_session *offer,
						const pjmedia_sdp_session *answer,
						pjsip_dialog *dlg,
						pjsip_endpoint *endpt,
						pjsip_tx_data **tdata);

/**
 * Create UAS invite session for the specified dialog in dlg. Application 
 * SHOULD call the verification function before calling this function, 
 * to ensure that it can create the session successfully.
 *
 * @param dlg		The dialog to be used.
 * @param rdata		Application MUST specify the received INVITE request 
 *			in rdata. The invite session needs to inspect the 
 *			received request to see if the request contains 
 *			features that it supports.
 * @param local_sdp	If application has determined its media capability, 
 *			it can specify this capability in this argument. 
 *			If SDP is received in the initial INVITE, the UAS 
 *			capability specified in this argument doesn't have to
 *			match the received offer; the SDP negotiator is able 
 *			to rearrange the media lines in the answer so that it
 *			matches the offer. 
 * @param options	The options argument is bitmask combination of SIP 
 *			features in pjsip_inv_options enumeration.
 * @param p_inv		Pointer to receive the newly created invite session.
 *
 * @return		On successful, the invite session will be put in 
 *			p_inv argument and the function will return PJ_SUCCESS.
 *			Otherwise the appropriate error status will be returned 
 *			on failure.
 */
PJ_DECL(pj_status_t) pjsip_inv_create_uas(pjsip_dialog *dlg,
					  pjsip_rx_data *rdata,
					  const pjmedia_sdp_session *local_sdp,
					  unsigned options,
					  pjsip_inv_session **p_inv);


/**
 * Forcefully terminate and destroy INVITE session, regardless of
 * the state of the session. Note that this function should only be used
 * when there is failure in the INVITE session creation. After the
 * invite session has been created and initialized, normally application
 * SHOULD use #pjsip_inv_end_session() to end the INVITE session instead.
 *
 * Note also that this function may terminate the underlying dialog, if
 * there are no other sessions in the dialog.
 *
 * @param inv		The invite session.
 * @param st_code	Status code for the reason of the termination.
 * @param notify	If set to non-zero, then on_state_changed() 
 *			callback will be called.
 *
 * @return		PJ_SUCCESS if the INVITE session has been
 *			terminated.
 */
PJ_DECL(pj_status_t) pjsip_inv_terminate( pjsip_inv_session *inv,
				          int st_code,
					  pj_bool_t notify );


/**
 * Create the initial INVITE request for this session. This function can only 
 * be called for UAC session. If local media capability is specified when 
 * the invite session was created, then this function will put an SDP offer 
 * in the outgoing INVITE request. Otherwise the outgoing request will not 
 * contain SDP body.
 *
 * @param inv		The UAC invite session.
 * @param p_tdata	The initial INVITE request will be put in this 
 *			argument if it can be created successfully.
 *
 * @return		PJ_SUCCESS if the INVITE request can be created.
 */
PJ_DECL(pj_status_t) pjsip_inv_invite( pjsip_inv_session *inv,
				       pjsip_tx_data **p_tdata );


/**
 * Create the initial response message for the incoming INVITE request in
 * rdata with  status code st_code and optional status text st_text. Use
 * #pjsip_inv_answer() to create subsequent response message.
 */
PJ_DECL(pj_status_t) pjsip_inv_initial_answer(	pjsip_inv_session *inv,
						pjsip_rx_data *rdata,
						int st_code,
						const pj_str_t *st_text,
						const pjmedia_sdp_session *sdp,
						pjsip_tx_data **p_tdata);

/**
 * Create a response message to the initial INVITE request. This function
 * can only be called for the initial INVITE request, as subsequent
 * re-INVITE request will be answered automatically.
 *
 * @param inv		The UAS invite session.
 * @param st_code	The st_code contains the status code to be sent, 
 *			which may be a provisional or final response. 
 * @param st_text	If custom status text is desired, application can 
 *			specify the text in st_text; otherwise if this 
 *			argument is NULL, default status text will be used.
 * @param local_sdp	If application has specified its media capability
 *			during creation of UAS invite session, the local_sdp
 *			argument MUST be NULL. This is because application 
 *			can not perform more than one SDP offer/answer session
 *			in a single INVITE transaction.
 *			If application has not specified its media capability 
 *			during creation of UAS invite session, it MAY or MUST
 *			specify its capability in local_sdp argument, 
 *			depending whether st_code indicates a 2xx final 
 *			response.
 * @param p_tdata	Pointer to receive the response message created by
 *			this function.
 *
 * @return		PJ_SUCCESS if response message was created
 *			successfully.
 */
PJ_DECL(pj_status_t) pjsip_inv_answer(	pjsip_inv_session *inv,
					int st_code,
					const pj_str_t *st_text,
					const pjmedia_sdp_session *local_sdp,
					pjsip_tx_data **p_tdata );


/**
 * Set local answer to respond to remote SDP offer, to be carried by 
 * subsequent response (or request).
 *
 * @param inv		The invite session.
 * @param sdp		The SDP description which will be set as answer
 *			to remote.
 *
 * @return		PJ_SUCCESS if local answer can be accepted by
 *			SDP negotiator.
 */
PJ_DECL(pj_status_t) pjsip_inv_set_sdp_answer(pjsip_inv_session *inv,
					      const pjmedia_sdp_session *sdp );


/**
 * Create a SIP message to initiate invite session termination. Depending on 
 * the state of the session, this function may return CANCEL request, 
 * a non-2xx final response, or a BYE request. If the session has not answered
 * the incoming INVITE, this function creates the non-2xx final response with 
 * the specified status code in st_code and optional status text in st_text.
 *
 * @param inv		The invite session.
 * @param st_code	Status code to be used for terminating the session.
 * @param st_text	Optional status text.
 * @param p_tdata	Pointer to receive the message to be created.
 *
 * @return		PJ_SUCCESS if termination message can be created.
 */
PJ_DECL(pj_status_t) pjsip_inv_end_session( pjsip_inv_session *inv,
					    int st_code,
					    const pj_str_t *st_text,
					    pjsip_tx_data **p_tdata );



/**
 * Create a re-INVITE request. 
 *
 * @param inv		The invite session.
 * @param new_contact	If application wants to update its local contact and
 *			inform peer to perform target refresh with a new 
 *			contact, it can specify the new contact in this 
 *			argument; otherwise this argument must be NULL.
 * @param new_offer	Application MAY initiate a new SDP offer/answer 
 *			session in the request when there is no pending 
 *			answer to be sent or received. It can detect this 
 *			condition by observing the state of the SDP 
 *			negotiator of the invite session. If new offer 
 *			should be sent to remote, the offer must be specified
 *			in this argument, otherwise it must be NULL.
 * @param p_tdata	Pointer to receive the re-INVITE request message to
 *			be created.
 *
 * @return		PJ_SUCCESS if a re-INVITE request with the specified
 *			characteristics (e.g. to contain new offer) can be 
 *			created.
 */
PJ_DECL(pj_status_t) pjsip_inv_reinvite(pjsip_inv_session *inv,
					const pj_str_t *new_contact,
					const pjmedia_sdp_session *new_offer,
					pjsip_tx_data **p_tdata );



/**
 * Create an UPDATE request to initiate new SDP offer.
 *
 * @param inv		The invite session.
 * @param new_contact	If application wants to update its local contact
 *			and inform peer to perform target refresh with a new
 *			contact, it can specify the new contact in this 
 *			argument; otherwise this argument must be NULL.
 * @param offer		Offer to be sent to remote. This argument is
 *			mandatory.
 * @param p_tdata	Pointer to receive the UPDATE request message to
 *			be created.
 *
 * @return		PJ_SUCCESS if a UPDATE request with the specified
 *			characteristics (e.g. to contain new offer) can be 
 *			created.
 */
PJ_DECL(pj_status_t) pjsip_inv_update (	pjsip_inv_session *inv,
					const pj_str_t *new_contact,
					const pjmedia_sdp_session *offer,
					pjsip_tx_data **p_tdata );


/**
 * Create an ACK request. Normally ACK request transmission is handled
 * by the framework. Application only needs to use this function if it
 * handles the ACK transmission manually, by overriding \a on_send_ack()
 * callback in #pjsip_inv_callback.
 *
 * Note that if the invite session has a pending offer to be answered 
 * (for example when the last 2xx response to INVITE contains an offer), 
 * application MUST have set the SDP answer with #pjsip_create_sdp_body()
 * prior to creating the ACK request. In this case, the ACK request
 * will be added with SDP message body.
 *
 * @param inv		The invite session.
 * @param cseq		Mandatory argument to specify the CSeq of the
 *			ACK request. This value MUST match the value
 *			of the INVITE transaction to be acknowledged.
 * @param p_tdata	Pointer to receive the ACK request message to
 *			be created.
 *
 * @return		PJ_SUCCESS if ACK request has been created.
 */
PJ_DECL(pj_status_t) pjsip_inv_create_ack(pjsip_inv_session *inv,
					  int cseq,
					  pjsip_tx_data **p_tdata);


/**
 * Send request or response message in tdata. 
 *
 * @param inv		The invite session.
 * @param tdata		The message to be sent.
 *
 * @return		PJ_SUCCESS if transaction can be initiated 
 *			successfully to send this message. Note that the
 *			actual final state of the transaction itself will
 *			be reported later, in on_tsx_state_changed()
 *			callback.
 */
PJ_DECL(pj_status_t) pjsip_inv_send_msg(pjsip_inv_session *inv,
					pjsip_tx_data *tdata);


/**
 * Get the invite session for the dialog, if any.
 *
 * @param dlg		The dialog which invite session is being queried.
 *
 * @return		The invite session instance which has been 
 *			associated with this dialog, or NULL.
 */
PJ_DECL(pjsip_inv_session*) pjsip_dlg_get_inv_session(pjsip_dialog *dlg);

/**
 * Get the invite session instance associated with transaction tsx, if any.
 *
 * @param tsx		The transaction, which invite session is being 
 *			queried.
 *
 * @return		The invite session instance which has been 
 *			associated with this transaction, or NULL.
 */
PJ_DECL(pjsip_inv_session*) pjsip_tsx_get_inv_session(pjsip_transaction *tsx);


/**
 * Get state names for INVITE session state.
 *
 * @param state		The invite state.
 *
 * @return		String describing the state.
 */
PJ_DECL(const char *) pjsip_inv_state_name(pjsip_inv_state state);


/**
 * This is a utility function to create SIP body for SDP content.
 *
 * @param pool		Pool to allocate memory.
 * @param sdp		SDP session to be put in the SIP message body.
 * @param p_body	Pointer to receive SIP message body containing
 *			the SDP session.
 *
 * @return		PJ_SUCCESS on success.
 */
PJ_DECL(pj_status_t) pjsip_create_sdp_body(pj_pool_t *pool,
					   pjmedia_sdp_session *sdp,
					   pjsip_msg_body **p_body);


PJ_END_DECL

/**
 * @}
 */


#endif	/* __SIP_INVITE_SESSION_H__ */
