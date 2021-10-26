#if !defined(_SETIRQ_H_)
#define _SETIRQ_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-03-25 16:38:19 #$
  $Revision:: 830              $

********************************************/

#define AIC_BASE_ADDR 0xFFFFF000
#define MAX_IRQ_REGISTERS 32
#define AIC_SIZE 0x1000
#define POSITIVE_EDGE_IRQ 0x60
#define NEGATIVE_EDGE_IRQ 0x20
#define HIGH_LEVEL_IRQ 0x40
#define LOW_LEVEL_IRQ 0x00

#define POSITIVE_EDGE_STR "Positive Edge"
#define NEGATIVE_EDGE_STR "Negative Edge"
#define HIGH_LEVEL_STR    "High Level"
#define LOW_LEVEL_STR     "Low Level"

#define PRIORITY_7 0x7
#define PRIORITY_6 0x6
#define PRIORITY_5 0x5
#define PRIORITY_4 0x4
#define PRIORITY_3 0x3
#define PRIORITY_2 0x2
#define PRIORITY_1 0x1
#define PRIORITY_0 0x0

#define AIC_PENDING_REGISTER 0x43
#define FAST_PENDING 0x08000000
#define SLOW_PENDING 0x04000000
#define PPS_PENDING  0x02000000

#define AIC_CLEAR_REGISTER 0x4A
#define CLEAR_PENDING_IRQS (FAST_PENDING | SLOW_PENDING | PPS_PENDING)


#define FAST_IRQ_OFFSET 0x1B
#define PPS_IRQ_OFFSET 0x19
#define SLOW_IRQ_OFFSET 0x1A

#define FAST_IRQ_NUMBER 27
#define FAST_IRQ_NAME "FastIrq"
#define FAST_IRQ_SEMAPHORE "FastIrqSem"
#define FAST_IRQ_TASK_NAME "FastIrqTask"
/* This one for test (count the T1 Irq) */
#define FAST_IRQ_TEST_SEMAPHORE "T1IrqTest"

#define SLOW_IRQ_NUMBER 26
#define SLOW_IRQ_NAME "SlowIrq"
#define SLOW_IRQ_SEMAPHORE "SlowIrqSem"
#define SLOW_IRQ_TASK_NAME "SlowIrqTask"
/* This one for test (Count the Slow Irqs) */
#define SLOW_IRQ_TEST_SEMAPHORE "MuIrqTest"


#define PPS_IRQ_NUMBER  25
#define PPS_IRQ_NAME "PpsIrq"
#define PPS_IRQ_TASK_NAME "PpsIrqTask"

#endif
