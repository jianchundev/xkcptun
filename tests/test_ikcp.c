#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ikcp.h"

typedef struct {
    int called;
    int last_len;
    int total_bytes;
    int packets;
} output_ctx;

static int dummy_output(const char *buf, int len, ikcpcb *kcp, void *user) {
    (void)kcp;
    output_ctx *ctx = (output_ctx*)user;
    if (ctx) {
        ctx->called = 1;
        ctx->last_len = len;
        ctx->total_bytes += len;
        ctx->packets += 1;
    }
    return len; // pretend we sent all bytes successfully
}

static void test_create_defaults(void) {
    ikcpcb *kcp = ikcp_create(123, NULL);
    assert(kcp != NULL);
    assert(kcp->conv == 123);
    assert(kcp->mss > 0);
    assert(kcp->mss < kcp->mtu);
    assert(kcp->interval > 0);
    assert(kcp->nodelay == 0);
    ikcp_release(kcp);
}

static void test_setmtu(void) {
    ikcpcb *k = ikcp_create(1, NULL);
    // Derive protocol overhead from defaults so we don't rely on internal constants
    int default_overhead = (int)k->mtu - (int)k->mss;
    int r = ikcp_setmtu(k, 1200);
    assert(r == 0);
    assert(k->mtu == 1200);
    assert(k->mss == 1200 - default_overhead);
    r = ikcp_setmtu(k, 20);
    assert(r < 0); // too small should fail
    ikcp_release(k);
}

static void test_interval(void) {
    ikcpcb *k = ikcp_create(1, NULL);
    ikcp_interval(k, 5); // below minimum clamps to 10
    assert(k->interval == 10);
    ikcp_interval(k, 6000); // above maximum clamps to 5000
    assert(k->interval == 5000);
    ikcp_interval(k, 100);
    assert(k->interval == 100);
    ikcp_release(k);
}

static void test_nodelay(void) {
    ikcpcb *k = ikcp_create(1, NULL);
    ikcp_nodelay(k, 1, 20, 2, 1);
    assert(k->nodelay == 1);
    assert(k->rx_minrto < 100); // enabling nodelay lowers min RTO
    assert(k->interval == 20);
    assert(k->fastresend == 2);
    assert(k->nocwnd == 1);
    ikcp_nodelay(k, 0, 0, 0, 0);
    assert(k->rx_minrto >= 100); // reset to normal RTO minimum
    ikcp_release(k);
}

static void test_wndsize_waitsnd(void) {
    ikcpcb *k = ikcp_create(1, NULL);
    ikcp_wndsize(k, 64, 128);
    assert(k->snd_wnd == 64);
    assert(k->rcv_wnd == 128);
    assert(ikcp_waitsnd(k) == 0);
    const char *buf = "abcdefghi"; // 9 bytes
    int sret = ikcp_send(k, buf, 9);
    assert(sret == 0);
    assert(ikcp_waitsnd(k) >= 1);
    ikcp_release(k);
}

static void test_getconv(void) {
    unsigned char b[4];
    // little-endian representation of 0x11223344
    b[0] = 0x44; b[1] = 0x33; b[2] = 0x22; b[3] = 0x11;
    IUINT32 v = ikcp_getconv(b);
    assert(v == 0x11223344u);
}

static void test_send_flush_output(void) {
    output_ctx ctx = {0};
    ikcpcb *k = ikcp_create(7, &ctx);
    ikcp_setoutput(k, dummy_output);

    const char *msg = "hello world";
    int rc = ikcp_send(k, msg, (int)strlen(msg));
    assert(rc == 0);

    // First update initializes and sets cwnd to 1 at the end
    ikcp_update(k, 0);
    // Second update after interval should actually move data and call output
    ikcp_update(k, 100);

    assert(ctx.called == 1);
    assert(ctx.total_bytes > 0);

    ikcp_release(k);
}

int main(void) {
    test_create_defaults();
    test_setmtu();
    test_interval();
    test_nodelay();
    test_wndsize_waitsnd();
    test_getconv();
    test_send_flush_output();
    printf("All ikcp basic tests passed.\n");
    return 0;
}
