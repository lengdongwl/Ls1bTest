/*
 * mytask.h
 *
 * created: 2022/11/23
 *  author: 
 */

#ifndef _MYTASK_H
#define _MYTASK_H

#ifdef __cplusplus
extern "C" {
#endif

void func_task_HMC5883L(void* arg);
void func_task_ultrasonic(void* arg);
void func_task_tempratrue(void* arg);
void func_task_bh1750(void* arg);

void func_task_YY();

void func_task_SMG1();
void func_task_SMG2();

void func_task_Fun(void *arg);

void func_task_Motor(void *arg); //电机检测速度 防止干扰发生任务优先级必须最高

void func_task_img();
void func_task_color();

void func_task_ADC(void *arg);

void func_task_Other1(void *arg);
void func_task_Other2(void *arg);
void func_task_Other3(void *arg);

void func_task_Wave(void *arg);


#ifdef __cplusplus
}
#endif

#endif // _MYTASK_H

