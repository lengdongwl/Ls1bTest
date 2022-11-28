/*
 * install_3th_libraries.c
 */

#include "bsp.h"

/*
 * yaffs2 �ļ�ϵͳ
 */
#ifdef USE_YAFFS2
#include "../yaffs2/port/ls1x_yaffs.h"
#endif

/*
 * lwIP 1.4.1
 */
#ifdef USE_LWIP
extern void lwip_init(void);
extern void ls1x_initialize_lwip(unsigned char *ip0, unsigned char *ip1);
#endif

/*
 * ftp ������
 */
#ifdef USE_FTPD
extern int start_ftpd_server(void);
#endif

/*
 * modbus Э���
 */
#ifdef USE_MODBUS
//
#endif

/*
 * lvgl GUI library
 */
#ifdef USE_LVGL
extern void lv_init(void);  			// in "lv_obj.c"
extern void lv_port_disp_init(void);  	// in "lv_port_disp.c"
extern void lv_port_indev_init(void);  	// in "lv_port_indev.c"
extern void lv_port_fs_init(void);     	// in "lv_port_fs.c"
#endif

//-----------------------------------------------------------------------------
// Initialize Libraries By user Selected
//-----------------------------------------------------------------------------

int install_3th_libraries(void)
{
    /*
     * yaffs2 �ļ�ϵͳ
     */
    #ifdef USE_YAFFS2
        yaffs_startup_and_mount(RYFS_MOUNTED_FS_NAME);
    #endif

    /*
     * lwIP 1.4.1
     */
    #ifdef USE_LWIP
      #ifdef OS_NONE
        lwip_init();                        // Initilaize the LwIP stack
      #endif
        ls1x_initialize_lwip(NULL, NULL);   // Initilaize the LwIP & GMAC0 glue
    #endif

#if BSP_USE_OS
    /*
     * ftp ������
     */
    #ifdef USE_FTPD
        start_ftpd_server();
    #endif

#endif

	/*
	 * modbus Э���
	 */
	#ifdef USE_MODBUS
        modbus_init(100);      		// 100HZ = 10ms
	#endif

	/*
	 * lvgl GUI library
	 */
	#ifdef USE_LVGL
        lv_init();					// ϵͳ��ʼ��

        lv_port_disp_init();        // ��ʾ�ӿ�

	  #if defined(XPT2046_DRV) || defined(GT1151_DRV)
        lv_port_indev_init();       // ����ӿ�
	  #endif

	  #ifdef USE_YAFFS2
        lv_port_fs_init();          // �ļ��ӿ�
	  #endif
	#endif

    return 0;
}


