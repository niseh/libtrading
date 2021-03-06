#ifndef LIBTRADING_FIX_SESSION_H
#define LIBTRADING_FIX_SESSION_H

#include "libtrading/proto/fix_message.h"

#include "libtrading/buffer.h"

#include <stdbool.h>

#define RECV_BUFFER_SIZE	4096UL
#define FIX_TX_HEAD_BUFFER_SIZE	FIX_MAX_HEAD_LEN
#define FIX_TX_BODY_BUFFER_SIZE	FIX_MAX_BODY_LEN

struct fix_message;

enum fix_version {
	FIXT_1_1,
	FIX_4_4,
	FIX_4_3,
	FIX_4_2,
	FIX_4_1,
	FIX_4_0,
};

struct fix_dialect {
	enum fix_version	version;
	enum fix_type		(*tag_type)(int tag);
};

extern struct fix_dialect	fix_dialects[];

struct fix_session_cfg {
	char			sender_comp_id[32];
	char			target_comp_id[32];
	int			heartbtint;
	struct fix_dialect	*dialect;
	int			sockfd;
};

struct fix_session {
	struct fix_dialect		*dialect;
	int				sockfd;
	bool				active;
	const char			*begin_string;
	const char			*sender_comp_id;
	const char			*target_comp_id;

	unsigned long			in_msg_seq_num;
	unsigned long			out_msg_seq_num;

	struct buffer			*rx_buffer;
	struct buffer			*tx_head_buffer;
	struct buffer			*tx_body_buffer;

	struct fix_message		*rx_message;

	int				heartbtint;

	struct timespec			now;
	char				str_now[64];

	struct timespec			rx_timestamp;
	struct timespec			tx_timestamp;

	char				testreqid[64];
	struct timespec			tr_timestamp;
	int				tr_pending;
};

static inline bool fix_msg_expected(struct fix_session *session, struct fix_message *msg)
{
	return msg->msg_seq_num == session->in_msg_seq_num || fix_message_type_is(msg, FIX_MSG_TYPE_SEQUENCE_RESET);
}

struct fix_session_cfg *fix_session_cfg_new(const char *sender_comp_id, const char *target_comp_id, int heartbtint, const char *dialect, int sockfd);
struct fix_session *fix_session_new(struct fix_session_cfg *cfg);
void fix_session_free(struct fix_session *self);
int fix_session_time_update(struct fix_session *self);
int fix_session_send(struct fix_session *self, struct fix_message *msg, int flags);
struct fix_message *fix_session_recv(struct fix_session *self, int flags);
bool fix_session_keepalive(struct fix_session *session, struct timespec *now);
bool fix_session_admin(struct fix_session *session, struct fix_message *msg);
int fix_session_logon(struct fix_session *session);
int fix_session_logout(struct fix_session *session, const char *text);
int fix_session_heartbeat(struct fix_session *session, const char *test_req_id);
int fix_session_test_request(struct fix_session *session);
int fix_session_resend_request(struct fix_session *session, unsigned long bgn, unsigned long end);
int fix_session_reject(struct fix_session *session, unsigned long refseqnum, char *text);
int fix_session_sequence_reset(struct fix_session *session, unsigned long msg_seq_num, unsigned long new_seq_num, bool gap_fill);
int fix_session_order_cancel_replace(struct fix_session *session, struct fix_field *fields, long nr_fields);
int fix_session_new_order_single(struct fix_session *session, struct fix_field* fields, long nr_fields);
int fix_session_execution_report(struct fix_session *session, struct fix_field *fields, long nr_fields);

#define	FIX_FLAG_PRESERVE_MSG_NUM	0x01

#endif
