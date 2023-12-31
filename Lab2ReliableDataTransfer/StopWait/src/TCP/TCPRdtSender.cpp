#include "TCPRdtSender.h"

using namespace dzh;

TCPRdtSender::TCPRdtSender() {
    window_base = 1;
    next_seqnum_to_send = 1;
    waiting_state = false;
    window_idx = 0;
    resend_cnt = 0;
}

TCPRdtSender::~TCPRdtSender() noexcept {

}

bool TCPRdtSender::getWaitingState() {
    return this->waiting_state;
}

Packet TCPRdtSender::make_pkt(int nextseqnum, const Message &message) {
    Packet packet;
    packet.acknum = -1;
    packet.seqnum = nextseqnum;
    packet.checksum = 0;
    memcpy(packet.payload, message.data, sizeof(message.data));
    packet.checksum = pUtils->calculateCheckSum(packet);
    return packet;
}

/**
 * get data from app layer, send to net layer
 * @param message
 * @return
 */
bool TCPRdtSender::send(const Message &message) {
    // window is not full
    if(next_seqnum_to_send < window_base + WIN_LENGTH){
        // make packet
        window[window_idx] = make_pkt(next_seqnum_to_send, message);
        // send packet
        pUtils->printPacket("发送方发送报文", window[window_idx]);
        pns->sendToNetworkLayer(RECEIVER, window[window_idx]);
        if(window_base == next_seqnum_to_send){
            pns->startTimer(SENDER, Configuration::TIME_OUT, window[window_idx].seqnum);
        }
        window_idx++;
        if(window_idx >= WIN_LENGTH){
            waiting_state = true;
        }
        next_seqnum_to_send++;
        return true;
    } else{ // failed to send, window is full , waiting for the ack
        waiting_state = true;
        return false;
    }
}

/**
 * get ack from net layer
 * @param ackPkt
 */
void TCPRdtSender::receive(const Packet &ackPkt) {
    // seqnum > base
//    if(window_idx > 0){
    // do rdt check
    checksum = pUtils->calculateCheckSum(ackPkt);
    if(checksum == ackPkt.checksum && ackPkt.acknum >= window_base){    // >= !!!
        int forward_num = ackPkt.acknum - window_base + 1;
//        int forward_num = ackPkt.acknum - window_base;
        // reset base
        window_base = ackPkt.acknum + 1;
//        if(last_ack_pkt.seqnum == ackPkt.acknum){
//            resend_cnt++;
//            if(resend_cnt == 3){
//                pns->stopTimer(SENDER, last_ack_pkt.seqnum);
//                pns->sendToNetworkLayer(RECEIVER,last_ack_pkt);//将第一个报文发送给接收方
//                pUtils->printPacket("冗余ACK*3，快速重传当前窗口第一个报文",last_ack_pkt);
//                pns->startTimer(SENDER,Configuration::TIME_OUT, last_ack_pkt.seqnum);
//                printf("冗余ACK%d *3",ackPkt.acknum);
//                resend_cnt = 0;
//                return;
//            }
//        } else{
//            resend_cnt = 0;
//        }
        if(last_ack_pkt.seqnum == ackPkt.acknum){
            resend_cnt++;
            if(resend_cnt == 3){
                pns->stopTimer(SENDER, last_ack_pkt.seqnum);
                pns->sendToNetworkLayer(RECEIVER,last_ack_pkt);//将第一个报文发送给接收方
                pUtils->printPacket("冗余ACK*3，快速重传当前窗口第一个报文",last_ack_pkt);
                pns->startTimer(SENDER,Configuration::TIME_OUT, last_ack_pkt.seqnum);
                printf("冗余ACK%d *3",ackPkt.acknum);
                resend_cnt = 0;
                return;
            }
        } else{
            resend_cnt = 0;
        }
        last_ack_pkt = ackPkt;
        // all pkt are acked
        if(window_base == next_seqnum_to_send){
            pUtils->printPacket("发送方正确收到确认", this->window[0]);
            pns->stopTimer(SENDER, this->window[0].seqnum);
        } else{ // cumulative confirm.
            pUtils->printPacket("发送方没有正确收到确认，重发上次发送的报文", this->window[0]);
            pns->stopTimer(SENDER, this->window[0].seqnum);
            pns->startTimer(SENDER, Configuration::TIME_OUT, this->window[forward_num].seqnum);
        }
        for(const auto & i : window){
            pUtils->printPacket("SEND（0）窗口内容：",i);
        }
        // sliding
        for(int i=forward_num;i<=window_idx && i < WIN_LENGTH;i++){
            window[i-forward_num] = window[i];
        }
//        for(int i=window_idx-forward_num+1;i<WIN_LENGTH;i++){
//            for(auto & p: window[i].payload) p = '.';
//        }
        for(const auto & i : window){
            pUtils->printPacket("SEND（1）窗口内容：",i);
        }
        // reset idx
        window_idx -= forward_num;
        waiting_state = false;
    }
//    }
}

void TCPRdtSender::timeoutHandler(int seqNum) {
    pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->window[0]);
    // restart timer
    pns->stopTimer(SENDER, window[0].seqnum);
    pns->startTimer(SENDER, Configuration::TIME_OUT, window[0].seqnum);
    // resend
    for(int i=0;i<window_idx;i++){
        pns->sendToNetworkLayer(RECEIVER, window[i]);
    }
}