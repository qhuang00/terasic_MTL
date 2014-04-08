/*
 * Copyright (C) 2013 Altera Corporation
 * Copyright (C) 2011 Thomas Chou
 * Copyright (C) 2011 Walter Goossens
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file COPYING in the main directory of this
 * archive for more details.
 */

#include <linux/init.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/sys_soc.h>
#include <linux/io.h>

#define NIOS2_ID_DEFAULT		(0x1)
#define NIOS2_REVISION_DEFAULT		(0x1)

static struct of_device_id altera_of_bus_ids[] __initdata = {
	{ .compatible = "simple-bus", },
	{ .compatible = "altr,avalon", },
	{}
};

static void __init nios2_soc_device_init(void)
{
	struct device_node *root;
	struct soc_device *soc_dev;
	struct soc_device_attribute *soc_dev_attr;
	const char *machine;
	int err;

	root = of_find_node_by_path("/");
	if (!root)
		return;

	err = of_property_read_string(root, "model", &machine);
	if (err)
		return;

	of_node_put(root);

	soc_dev_attr = kzalloc(sizeof(*soc_dev_attr), GFP_KERNEL);
	if (!soc_dev_attr)
		return;

	soc_dev_attr->soc_id = kasprintf(GFP_KERNEL, "%u", NIOS2_ID_DEFAULT);
	soc_dev_attr->revision = kasprintf(GFP_KERNEL, "%d",
		NIOS2_REVISION_DEFAULT);
	soc_dev_attr->machine = kasprintf(GFP_KERNEL, "%s", machine);
	soc_dev_attr->family = "Nios II";

	soc_dev = soc_device_register(soc_dev_attr);
	if (IS_ERR_OR_NULL(soc_dev)) {
		kfree(soc_dev_attr->soc_id);
		kfree(soc_dev_attr->machine);
		kfree(soc_dev_attr->revision);
		kfree(soc_dev_attr);
		return;
	}

	return;
}

static int __init nios2_device_probe(void)
{
	nios2_soc_device_init();

	of_platform_bus_probe(NULL, altera_of_bus_ids, NULL);
	return 0;
}

device_initcall(nios2_device_probe);
