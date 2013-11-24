#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/sii9234.h>

#include "MHD_SiI9234.h"
#include "sii9234_tpi_regs.h"

#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>

#include <linux/slab.h>

#define SUBJECT "MHL_DRIVER"

#define SII_DEV_DBG(format, ...)\
	pr_info("[ "SUBJECT " (%s,%d) ] " format "\n",\
		__func__, __LINE__, ## __VA_ARGS__);

struct i2c_client *SII9234_i2c_client;
struct i2c_client *SII9234A_i2c_client;
struct i2c_client *SII9234B_i2c_client;
struct i2c_client *SII9234C_i2c_client;

static struct i2c_device_id SII9234_id[] = {
	{"SII9234", 0},
	{}
};

static struct i2c_device_id SII9234A_id[] = {
	{"SII9234A", 0},
	{}
};

static struct i2c_device_id SII9234B_id[] = {
	{"SII9234B", 0},
	{}
};

static struct i2c_device_id SII9234C_id[] = {
	{"SII9234C", 0},
	{}
};

struct SII9234_state {
	struct i2c_client *client;
};

static struct timer_list MHL_reg_check;

static u8 SII9234_i2c_read(struct i2c_client *client, u8 reg)
{
	u8 ret;

	if (!client) {
		SII_DEV_DBG("i2c not ready");
		return -EINVAL;
	}

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		SII_DEV_DBG("i2c read fail");
		return -EIO;
	}
	return ret;
}


static int SII9234_i2c_write(struct i2c_client *client, u8 reg, u8 data)
{
	if (!client) {
		SII_DEV_DBG("i2c not ready");
		return -EINVAL;
	}
	
	return i2c_smbus_write_byte_data(client, reg, data);
}

void SII9234_HW_Reset(struct i2c_client *client)
{
	struct sii9234_platform_data *pdata = client->dev.platform_data;
	SII_DEV_DBG("");
	if (pdata && pdata->hw_reset)
		pdata->hw_reset();
}

void SII9234_HW_Off(struct i2c_client *client)
{
	struct sii9234_platform_data *pdata = client->dev.platform_data;
	SII_DEV_DBG("");
	if (pdata && pdata->hw_off)
		pdata->hw_off();
}

int SII9234_HW_IsOn(struct i2c_client *client)
{
	struct sii9234_platform_data *pdata = client->dev.platform_data;
	SII_DEV_DBG("");
	if (pdata && pdata->hw_is_on)
		return pdata->hw_is_on();
	else 
		return 0;
}

#include "MHD_SiI9234.c"

void sii_9234_monitor(unsigned long arg)
{
	SII_DEV_DBG("");
	ReadIndexedRegister(INDEXED_PAGE_0, 0x81);
}

static void check_HDMI_signal(unsigned long arg)
{
	SII_DEV_DBG("");
	sii9234_tpi_init();
	
	MHL_reg_check.function = sii_9234_monitor;
	MHL_reg_check.expires = get_jiffies_64() + (HZ*3);
	add_timer(&MHL_reg_check);
}

static int SII9234_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct SII9234_state *state;

	SII_DEV_DBG("");

	state = kzalloc(sizeof(struct SII9234_state), GFP_KERNEL);
	if (!state) {
		pr_err("failed to allocate memory\n");
		return -ENOMEM;
	}
	
	state->client = client;
	i2c_set_clientdata(client, state);
	
	/* rest of the initialisation goes here. */
	
	pr_info("SII9234 attach success!!!\n");

	SII9234_i2c_client = client;
	
	return 0;
}

static int __devexit SII9234_remove(struct i2c_client *client)
{
	struct SII9234_state *state = i2c_get_clientdata(client);
	
	kfree(state);

	return 0;
}

static int SII9234A_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct SII9234_state *state;
	
	SII_DEV_DBG("");

	state = kzalloc(sizeof(struct SII9234_state), GFP_KERNEL);
	if (!state) {
		pr_err("failed to allocate memory\n");
		return -ENOMEM;
	}
	
	state->client = client;
	i2c_set_clientdata(client, state);
	
	/* rest of the initialisation goes here. */
	
	pr_info("SII9234A attach success!!!\n");

	SII9234A_i2c_client = client;

	return 0;
}

static int __devexit SII9234A_remove(struct i2c_client *client)
{
	struct SII9234_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}

static int SII9234B_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct SII9234_state *state;
	
	SII_DEV_DBG("");

	state = kzalloc(sizeof(struct SII9234_state), GFP_KERNEL);
	if (!state) {
		pr_err("failed to allocate memory\n");
		return -ENOMEM;
	}
	
	state->client = client;
	i2c_set_clientdata(client, state);
	
	/* rest of the initialisation goes here. */

	pr_info("SII9234B attach success!!!\n");

	SII9234B_i2c_client = client;
	
	return 0;
}

static int __devexit SII9234B_remove(struct i2c_client *client)
{
	struct SII9234_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}

static int SII9234C_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct SII9234_state *state;

	SII_DEV_DBG("");

	state = kzalloc(sizeof(struct SII9234_state), GFP_KERNEL);
	if (!state) {
		pr_err("failed to allocate memory\n");
		return -ENOMEM;
	}
	
	state->client = client;
	i2c_set_clientdata(client, state);
	
	/* rest of the initialisation goes here. */
	pr_info("SII9234C attach success!!!\n");

	SII9234C_i2c_client = client;

	return 0;
}

static int __devexit SII9234C_remove(struct i2c_client *client)
{
	struct SII9234_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}

struct i2c_driver SII9234_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234",
	},
	.id_table	= SII9234_id,
	.probe	= SII9234_i2c_probe,
	.remove	= __devexit_p(SII9234_remove),
	.command = NULL,
};

struct i2c_driver SII9234A_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234A",
	},
	.id_table	= SII9234A_id,
	.probe	= SII9234A_i2c_probe,
	.remove	= __devexit_p(SII9234A_remove),
	.command = NULL,
};

struct i2c_driver SII9234B_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234B",
	},
	.id_table	= SII9234B_id,
	.probe	= SII9234B_i2c_probe,
	.remove	= __devexit_p(SII9234B_remove),
	.command = NULL,
};

struct i2c_driver SII9234C_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234C",
	},
	.id_table	= SII9234C_id,
	.probe	= SII9234C_i2c_probe,
	.remove	= __devexit_p(SII9234C_remove),
	.command = NULL,
};


