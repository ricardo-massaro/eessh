/* kex_dh.c
 *
 * Diffie-Hellman key exchange for SSH conforming to RFC 4253 section 8
 */

#include <stdlib.h>
#include <string.h>

#include "ssh/kex_dh_i.h"

#include "ssh/kex_i.h"
#include "ssh/connection_i.h"
#include "ssh/hash_i.h"
#include "ssh/pubkey_i.h"
#include "ssh/connection_i.h"

#include "common/error.h"
#include "common/debug.h"
#include "ssh/debug.h"
#include "ssh/ssh_constants.h"
#include "crypto/dh.h"

#if !DEBUG_KEX
#include "common/disable_debug_i.h"
#endif

const static struct DH_ALGO {
  enum SSH_KEX_TYPE type;
  const char *gen;
  const char *modulus;
} dh_algos[] = {
  /*
   * diffie-hellman-group1-sha1
   * RFC 4243, section 8.1 (https://tools.ietf.org/html/rfc4253#section-8.1)
   * RFC 2409, section 6.2 (https://tools.ietf.org/html/rfc2409#section-6.2)
   */
  {
    SSH_KEX_DH_GROUP_1,
    "2",
    "FFFFFFFF" "FFFFFFFF" "C90FDAA2" "2168C234" "C4C6628B" "80DC1CD1"
    "29024E08" "8A67CC74" "020BBEA6" "3B139B22" "514A0879" "8E3404DD"
    "EF9519B3" "CD3A431B" "302B0A6D" "F25F1437" "4FE1356D" "6D51C245"
    "E485B576" "625E7EC6" "F44C42E9" "A637ED6B" "0BFF5CB6" "F406B7ED"
    "EE386BFB" "5A899FA5" "AE9F2411" "7C4B1FE6" "49286651" "ECE65381"
    "FFFFFFFF" "FFFFFFFF"
  },

  /*
   * diffie-hellman-group14-sha1
   * RFC 4243, section 8.2 (https://tools.ietf.org/html/rfc4253#section-8.2)
   * RFC 3526, section 3   (https://tools.ietf.org/html/rfc3526#section-3)
   */
  {
    SSH_KEX_DH_GROUP_14,
    "2",
    "FFFFFFFF" "FFFFFFFF" "C90FDAA2" "2168C234" "C4C6628B" "80DC1CD1"
    "29024E08" "8A67CC74" "020BBEA6" "3B139B22" "514A0879" "8E3404DD"
    "EF9519B3" "CD3A431B" "302B0A6D" "F25F1437" "4FE1356D" "6D51C245"
    "E485B576" "625E7EC6" "F44C42E9" "A637ED6B" "0BFF5CB6" "F406B7ED"
    "EE386BFB" "5A899FA5" "AE9F2411" "7C4B1FE6" "49286651" "ECE45B3D"
    "C2007CB8" "A163BF05" "98DA4836" "1C55D39A" "69163FA8" "FD24CF5F"
    "83655D23" "DCA3AD96" "1C62F356" "208552BB" "9ED52907" "7096966D"
    "670C354E" "4ABC9804" "F1746C08" "CA18217C" "32905E46" "2E36CE3B"
    "E39E772C" "180E8603" "9B2783A2" "EC07A28F" "B5C55DF0" "6F4C52C9"
    "DE2BCBF6" "95581718" "3995497C" "EA956AE5" "15D22618" "98FA0510"
    "15728E5A" "8AACAA68" "FFFFFFFF" "FFFFFFFF"
  },
};

/* send SSH_MSG_KEXDH_INIT packet */
static int dh_kex_send_init_msg(struct CRYPTO_DH *dh, struct SSH_CONN *conn)
{
  struct SSH_STRING e;
  struct SSH_BUFFER *pack;
 
  if (crypto_dh_get_pubkey(dh, &e) < 0)
    return -1;

  ssh_log("* sending SSH_MSG_KEXDH_INIT\n");

  pack = ssh_conn_new_packet(conn);
  if (pack == NULL)
    return -1;

  if (ssh_buf_write_u8(pack, SSH_MSG_KEXDH_INIT) < 0
      || ssh_buf_write_string(pack, &e) < 0)
    return -1;
  
  if (ssh_conn_send_packet(conn) < 0)
    return -1;
  return 0;
}

/*
 * Compute exchange hash according to RFC 4253 section 8
 * (see https://tools.ietf.org/html/rfc4253#section-8)
 */
static int dh_kex_hash(struct SSH_STRING *ret_hash, enum SSH_HASH_TYPE hash_type, const struct SSH_STRING *server_host_key,
                       const struct SSH_STRING *client_pubkey, const struct SSH_STRING *server_pubkey,
                       const struct SSH_STRING *shared_secret, struct SSH_CONN *conn, struct SSH_KEX *kex)
{
  struct SSH_BUFFER data;
  struct SSH_STRING data_str;
  struct SSH_STRING hash;
  struct SSH_VERSION_STRING *client_version;
  struct SSH_VERSION_STRING *server_version;
  uint8_t hash_data[SSH_HASH_MAX_LEN];

  client_version = ssh_conn_get_client_version_string(conn);
  server_version = ssh_conn_get_server_version_string(conn);
  
  data = ssh_buf_new();
  if (ssh_buf_write_data(&data, (uint8_t *) client_version->buf, client_version->len) < 0
      || ssh_buf_write_data(&data, (uint8_t *) server_version->buf, server_version->len) < 0
      || ssh_buf_write_buffer(&data, &kex->client_kexinit) < 0
      || ssh_buf_write_buffer(&data, &kex->server_kexinit) < 0
      || ssh_buf_write_string(&data, server_host_key) < 0
      || ssh_buf_write_string(&data, client_pubkey) < 0
      || ssh_buf_write_string(&data, server_pubkey) < 0
      || ssh_buf_write_string(&data, shared_secret) < 0)
    return -1;

  // hash data
  data_str = ssh_str_new_from_buffer(&data);
  hash = ssh_str_new(hash_data, 0);
  if (ssh_hash_compute(hash_type, &hash, &data_str) < 0) {
    ssh_buf_free(&data);
    return -1;
  }
  ssh_buf_free(&data);

  if (ssh_str_dup_string(ret_hash, &hash) < 0)
    return -1;
  
  return 0;
}

/* read SSH_MSG_KEXDH_REPLY message */
static int dh_kex_read_reply(struct CRYPTO_DH *dh, struct SSH_CONN *conn, struct SSH_KEX *kex)
{
  struct SSH_BUF_READER *pack;
  struct SSH_STRING server_host_key;
  struct SSH_STRING client_pubkey;
  struct SSH_STRING server_pubkey;
  struct SSH_STRING shared_secret;
  struct SSH_STRING server_hash_sig;
  struct SSH_STRING exchange_hash;
  
  pack = ssh_conn_recv_packet_skip_ignore(conn);
  if (pack == NULL)
    return -1;
  if (ssh_packet_get_type(pack) != SSH_MSG_KEXDH_REPLY) {
    ssh_set_error("unexpected packet type: %d (expected SSH_MSG_KEXDH_REPLY=%d)", ssh_packet_get_type(pack), SSH_MSG_KEXDH_REPLY);
    return -1;
  }
  ssh_log("* got SSH_MSG_KEXDH_REPLY\n");
  if (ssh_buf_read_u8(pack, NULL) < 0
      || ssh_buf_read_string(pack, &server_host_key) < 0
      || ssh_buf_read_string(pack, &server_pubkey) < 0
      || ssh_buf_read_string(pack, &server_hash_sig) < 0)
    return -1;
  //dump_string("* server_host_key", &server_host_key);
  //dump_string("* server_pubkey", &server_pubkey);
  //dump_string("* hash_sig", &server_hash_sig);

  // compute shared_secret
  if (crypto_dh_compute_key(dh, &shared_secret, &server_pubkey) < 0)
    return -1;

  // compute exchange_hash
  if (crypto_dh_get_pubkey(dh, &client_pubkey) < 0
      || dh_kex_hash(&exchange_hash, kex->hash_type, &server_host_key, &client_pubkey, &server_pubkey, &shared_secret, conn, kex) < 0) {
    ssh_str_free(&shared_secret);
    return -1;
  }

  // verify signature
  if (ssh_pubkey_verify_signature(kex->pubkey_type, &server_host_key, &server_hash_sig, &exchange_hash) < 0) {
    ssh_str_free(&shared_secret);
    ssh_str_free(&exchange_hash);
    return -1;
  }
  ssh_log("* server signature verified\n");

  // verify identity
  if (ssh_conn_check_server_identity(conn, &server_host_key) < 0) {
    ssh_str_free(&shared_secret);
    ssh_str_free(&exchange_hash);
    return -1;
  }
  ssh_log("* server identity verified\n");

  return ssh_kex_finish(conn, kex, &shared_secret, &exchange_hash);
}

const struct DH_ALGO *kex_dh_get_algo(enum SSH_KEX_TYPE type)
{
  int i;
  
  for (i = 0; i < sizeof(dh_algos)/sizeof(dh_algos[0]); i++) {
    if (dh_algos[i].type == type)
      return &dh_algos[i];
  }
  ssh_set_error("unknown kex DH type %d", type);
  return NULL;
}

int ssh_kex_dh_run(struct SSH_CONN *conn, struct SSH_KEX *kex)
{
  struct CRYPTO_DH *dh;
  const struct DH_ALGO *dh_algo;

  if ((dh_algo = kex_dh_get_algo(kex->type)) == NULL
      || (dh = crypto_dh_new(dh_algo->gen, dh_algo->modulus)) == NULL)
    return -1;

  if (dh_kex_send_init_msg(dh, conn) < 0
      || dh_kex_read_reply(dh, conn, kex) < 0) {
    crypto_dh_free(dh);
    return -1;
  }

  crypto_dh_free(dh);
  return 0;
}
