//
// Created by Davide Riemma on 10/06/21.
//

#ifndef PSSTR_COMMON_MAILBOX_H
#define PSSTR_COMMON_MAILBOX_H

/* Nome della mailbox utilizzata per la comunicazione tra i Voter Tasks ed il TBS */
#define TBS_MBX_NAME 100105

/* struttura che rappresenta un messaggio ricevuto dal server */
struct TBS_message_type
{
    unsigned int task_to_wake; //identificativo del task da risvagliare
};


#endif //PSSTR_COMMON_MAILBOX_H
