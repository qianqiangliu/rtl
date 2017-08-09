#include <stdio.h>
#include <stdlib.h>

#include "rtl_http_req.h"
#include "rtl_http_resp.h"
#include "rtl_socket.h"

int main()
{
	rtl_http_req_t *req;

	req = rtl_http_req_new(RTL_HTTP_REQ_TYPE_GET, "www.baidu.com", 80,
						   "/index.html");
	if (!req)
		return 1;

	rtl_http_hdr_set_value(req->headers, RTL_HTTP_HDR_Connection, "close");

	struct rtl_socket_connection *conn;

	if (!(conn = rtl_http_req_conn(req))) {
		rtl_http_req_destroy(req);
		return 1;
	}

	if (rtl_http_req_send(req, conn, NULL, 0) < 0) {
		rtl_http_req_discon(conn);
		rtl_http_req_destroy(req);
		return 1;
	}

	rtl_http_resp_t *resp;
	int ret = 0;

	resp = rtl_http_resp_new();
	if (!resp) {
		rtl_http_req_discon(conn);
		rtl_http_req_destroy(req);
		return 1;
	}
	if (rtl_http_resp_read_hdrs(resp, conn) < 0) {
		ret = 1;
		goto out;
	}

	printf("status_code = %d\n", rtl_http_resp_get_status_code(resp));
	printf("reason_phrase = %s\n", rtl_http_resp_get_reason_phrase(resp));

	printf("Connection: %s\n", rtl_http_hdr_get_value(resp->headers,
													  "Connection"));
	printf("Content-Length: %s\n", rtl_http_hdr_get_value(resp->headers,
														  "Content-Length"));

	rtl_http_resp_save_body_to_file(resp, conn, "index.html");

out:
	rtl_http_req_discon(conn);
	rtl_http_req_destroy(req);
	rtl_http_resp_destroy(resp);
	return ret;
}
