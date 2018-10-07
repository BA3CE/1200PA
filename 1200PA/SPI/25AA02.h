#ifndef __25AA02_H__
#define __25AA02_H__
void write_byte(INT8U adr, INT8U sz, const INT8U *buf);
void read_byte(INT8U address,INT8U sz, INT8U *buff);
INT8U read_status();
void write_status(INT8U data);
void write_disable();
void write_enable();
#endif