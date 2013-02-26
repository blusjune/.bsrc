/* tier sysfs attributes */
ssize_t tier_attr_show(struct device *dev, char *page,
		       ssize_t(*callback) (struct tier_device *, char *))
{
	struct tier_device *l, *lo = NULL;

	mutex_lock(&tier_devices_mutex);
	list_for_each_entry(l, &device_list, list)
	    if (disk_to_dev(l->gd) == dev) {
		lo = l;
		break;
	}
	mutex_unlock(&tier_devices_mutex);

	return lo ? callback(lo, page) : -EIO;
}

ssize_t tier_attr_store(struct device * dev, const char *page, size_t s,
			ssize_t(*callback) (struct tier_device *,
					    const char *, size_t))
{
	struct tier_device *l, *lo = NULL;

	mutex_lock(&tier_devices_mutex);
	list_for_each_entry(l, &device_list, list)
	    if (disk_to_dev(l->gd) == dev) {
		lo = l;
		break;
	}
	mutex_unlock(&tier_devices_mutex);

	return lo ? callback(lo, page, s) : -EIO;
}

#define TIER_ATTR_RO(_name) \
static ssize_t tier_attr_##_name##_show(struct tier_device *, char *);  \
static ssize_t tier_attr_do_show_##_name(struct device *d,              \
                                struct device_attribute *attr, char *b) \
{                                                                       \
        return tier_attr_show(d, b, tier_attr_##_name##_show);          \
}                                                                       \
static struct device_attribute tier_attr_##_name =                      \
        __ATTR(_name, S_IRUGO, tier_attr_do_show_##_name, NULL);

static ssize_t tier_attr_attacheddevices_show(struct tier_device *dev,
					      char *buf)
{
	return sprintf(buf, "%u\n", dev->attached_devices);
}

#define TIER_ATTR_WO(_name) \
static ssize_t tier_attr_##_name##_store(struct tier_device *, const char *, size_t);  \
static ssize_t tier_attr_do_store_##_name(struct device *d,                      \
                                struct device_attribute *attr, const char *b, size_t s)\
{                                                                                \
        return tier_attr_store(d, b, s, tier_attr_##_name##_store);              \
}                                                                                \
static struct device_attribute tier_attr_##_name =                               \
        __ATTR(_name, S_IRUGU, NULL, tier_attr_do_store_##_name);

#define TIER_ATTR_RW(_name) \
static ssize_t tier_attr_##_name##_store(struct tier_device *, const char *, size_t);  \
static ssize_t tier_attr_do_store_##_name(struct device *d,                      \
                                struct device_attribute *attr, const char *b, size_t s)\
{                                                                                \
        return tier_attr_store(d, b, s, tier_attr_##_name##_store);              \
}                                                                                \
static ssize_t tier_attr_do_show_##_name(struct device *d,              \
                                struct device_attribute *attr, char *b) \
{                                                                       \
        return tier_attr_show(d, b, tier_attr_##_name##_show);          \
}                                                                       \
static struct device_attribute tier_attr_##_name =                               \
        __ATTR(_name, (S_IRWXU ^ S_IXUSR) | S_IRGRP| S_IROTH,  tier_attr_do_show_##_name, tier_attr_do_store_##_name);

static ssize_t tier_attr_migration_enable_store(struct tier_device *dev,
						const char *buf, size_t s)
{
	if ('0' != buf[0] && '1' != buf[0])
		return s;
	if ('1' == buf[0]) {
		if (dev->migration_disabled) {
			dev->migration_disabled = 0;
			dev->resumeblockwalk = 0;
			if (0 == atomic_read(&dev->migrate)) {
				atomic_set(&dev->migrate, 1);
				wake_up(&dev->migrate_event);
			}
			TIERINFO("migration is enabled for %s", dev->devname);
		}
	} else {
		if (!dev->migration_disabled && 0 == atomic_read(&dev->migrate)) {
			dev->migration_disabled = 1;
			del_timer_sync(&dev->migrate_timer);
			TIERINFO("migration is disabled for %s", dev->devname);
		}
		dev->migration_disabled = 1;
	}
	return s;
}

static ssize_t tier_attr_barriers_store(struct tier_device *dev,
					const char *buf, size_t s)
{
	if ('0' != buf[0] && '1' != buf[0])
		return s;
	if ('0' == buf[0]) {
		if (dev->barrier) {
			dev->barrier = 0;
			TIERINFO("barriers are disabled");
		}
	} else {
		if (!dev->barrier) {
			dev->barrier = 1;
			TIERINFO("barriers are enabled");
		}
	}
	return s;
}

static ssize_t tier_attr_ptsync_store(struct tier_device *dev,
				      const char *buf, size_t s)
{
	if ('0' != buf[0] && '1' != buf[0])
		return s;
	if ('0' == buf[0]) {
		if (dev->ptsync) {
			dev->ptsync = 0;
			TIERINFO("pass-through sync is disabled");
		}
	} else {
		if (!dev->ptsync) {
			dev->ptsync = 1;
			TIERINFO("pass-through sync is enabled");
		}
	}
	return s;
}

static ssize_t tier_attr_resize_store(struct tier_device *dev,
				      const char *buf, size_t s)
{
	if ('1' != buf[0])
		return s;
	mutex_lock(&dev->cachelock);
	free_bitlists(dev);
	resize_tier(dev);
	load_bitlists(dev);
	mutex_unlock(&dev->cachelock);
	return s;
}

/* return the input NULL terminated */
static char *null_term_buf(const char *buf, size_t s)
{
	char *cpybuf;

	cpybuf = kzalloc(s + 1, GFP_KERNEL);
	if (NULL == cpybuf)
		return NULL;
	memcpy(cpybuf, buf, s);

	return cpybuf;
}

static ssize_t tier_attr_sequential_landing_store(struct tier_device *dev,
						  const char *buf, size_t s)
{
	int landdev;
	int res;
	char *cpybuf;

	cpybuf = null_term_buf(buf, s);
	if (NULL == cpybuf)
		return -ENOMEM;
	res = sscanf(cpybuf, "%i", &landdev);
	if (res != 1)
		return -ENOMSG;
	if (landdev >= dev->attached_devices)
		return -ENOMSG;
	if (landdev < 0)
		return -ENOMSG;
	dev->sequential_landing = landdev;
	kfree(cpybuf);
	return s;
}

static ssize_t tier_attr_migrate_verbose_store(struct tier_device *dev,
					       const char *buf, size_t s)
{
	if ('0' != buf[0] && '1' != buf[0])
		return s;
	if ('0' == buf[0]) {
		if (dev->migrate_verbose) {
			dev->migrate_verbose = 0;
			TIERINFO("migrate_verbose is disabled");
		}
	} else {
		if (!dev->ptsync) {
			dev->migrate_verbose = 1;
			TIERINFO("migrate_verbose is enabled");
		}
	}
	return s;
}

static ssize_t tier_attr_migration_policy_store(struct tier_device *dev,
						const char *buf, size_t s)
{
	int devicenr, res;
	unsigned int max_age;
	unsigned int hit_collecttime;

	char *a, *p, *cur;
	char *devicename;
	char *cpybuf;

	cpybuf = null_term_buf(buf, s);
	if (NULL == cpybuf)
		return -ENOMEM;

	p = strchr(cpybuf, ' ');
	if (NULL == p)
		goto end_error;
	a = kzalloc(p - cpybuf + 1, GFP_KERNEL);
	memcpy(a, cpybuf, p - cpybuf);
	res = sscanf(a, "%u", &devicenr);
	if (res != 1)
		goto end_error;
	if (devicenr < 0 || devicenr >= dev->attached_devices)
		goto end_error;
	kfree(a);
	a = p;

	while (a[0] == ' ')
		a++;
	p = strchr(a, ' ');
	if (NULL == p)
		goto end_error;
	devicename = kzalloc(p - cpybuf + 1, GFP_KERNEL);
	memcpy(devicename, a, p - a);
	if (0 !=
	    strcmp(devicename,
		   dev->backdev[devicenr]->tier_dta_file->f_dentry->
		   d_name.name)) {
		kfree(devicename);
		goto end_error;
	}
	kfree(devicename);

	a = p;
	while (a[0] == ' ')
		a++;
	p = strchr(a, ' ');
	cur = kzalloc(p - a, GFP_KERNEL);
	memcpy(cur, a, p - a + 1);
	res = sscanf(cur, "%u", &max_age);
	kfree(cur);
	if (res != 1)
		goto end_error;
	if (max_age < 0)
		goto end_error;

	a = p;
	while (a[0] == ' ')
		a++;
	res = sscanf(a, "%u", &hit_collecttime);
	if (res != 1)
		goto end_error;
	mutex_lock(&dev->cachelock);
	dev->backdev[devicenr]->dtapolicy.max_age = max_age;
	dev->backdev[devicenr]->dtapolicy.hit_collecttime = hit_collecttime;
	mutex_unlock(&dev->cachelock);
	kfree(cpybuf);
	return s;

      end_error:
	kfree(cpybuf);
	return -ENOMSG;
}

static ssize_t tier_attr_migration_interval_store(struct tier_device *dev,
						  const char *buf, size_t s)
{
	int res;
	u64 interval;
	char *cpybuf;

	cpybuf = null_term_buf(buf, s);
	if (NULL == cpybuf)
		return -ENOMEM;
	res = sscanf(cpybuf, "%llu", &interval);
	if (res == 1) {
		if (interval <= 0)
			return -ENOMSG;
		mutex_lock(&dev->cachelock);
		dev->migration_interval = interval;
		mod_timer(&dev->migrate_timer,
			  jiffies +
			  msecs_to_jiffies(dev->migration_interval * 1000));
		mutex_unlock(&dev->cachelock);
	} else
		s = -ENOMSG;
	kfree(cpybuf);
	return s;
}

static ssize_t tier_attr_migration_enable_show(struct tier_device *dev,
					       char *buf)
{
	return sprintf(buf, "%i\n", !dev->migration_disabled);
}

static ssize_t tier_attr_barriers_show(struct tier_device *dev, char *buf)
{
	return sprintf(buf, "%i\n", dev->barrier);
}

static ssize_t tier_attr_ptsync_show(struct tier_device *dev, char *buf)
{
	return sprintf(buf, "%i\n", dev->ptsync);
}

static ssize_t tier_attr_resize_show(struct tier_device *dev, char *buf)
{
	return sprintf(buf, "0\n");
}

static ssize_t tier_attr_sequential_landing_show(struct tier_device *dev,
						 char *buf)
{
	return sprintf(buf, "%i\n", dev->sequential_landing);
}

static ssize_t tier_attr_migrate_verbose_show(struct tier_device *dev,
					      char *buf)
{
	return sprintf(buf, "%i\n", dev->migrate_verbose);
}

static ssize_t tier_attr_migration_policy_show(struct tier_device *dev,
					       char *buf)
{
	char *msg = NULL;
	char *msg2;
	int count;

	for (count = 0; count < dev->attached_devices; count++) {
		if (NULL == msg) {
			msg2 =
			    as_sprintf
			    ("%7s %20s %15s %15s\n%7u %20s %15u %15u\n", "tier",
			     "device", "max_age", "hit_collecttime", count,
			     dev->backdev[count]->tier_dta_file->
			     f_dentry->d_name.name,
			     dev->backdev[count]->dtapolicy.max_age,
			     dev->backdev[count]->dtapolicy.hit_collecttime);
		} else {
			msg2 =
			    as_sprintf("%s%7u %20s %15u %15u\n", msg,
				       count,
				       dev->backdev[count]->
				       tier_dta_file->f_dentry->d_name.name,
				       dev->backdev[count]->dtapolicy.max_age,
				       dev->backdev[count]->
				       dtapolicy.hit_collecttime);
		}
		kfree(msg);
		msg = msg2;
	}
	return sprintf(buf, "%s\n", msg);
	kfree(msg);
}

static ssize_t tier_attr_migration_interval_show(struct tier_device *dev,
						 char *buf)
{
	return sprintf(buf, "%llu\n", dev->migration_interval);
}

static ssize_t tier_attr_numwrites_show(struct tier_device *dev, char *buf)
{
	return sprintf(buf, "sequential %llu random %llu\n",
		       dev->stats.seq_writes, dev->stats.rand_writes);
}

static ssize_t tier_attr_numreads_show(struct tier_device *dev, char *buf)
{
	return sprintf(buf, "sequential %llu random %llu\n",
		       dev->stats.seq_reads, dev->stats.rand_reads);
}

static ssize_t tier_attr_device_usage_show(struct tier_device *dev, char *buf)
{
	unsigned int count = 0;
	int res;
	u64 allocated;
	unsigned int lcount = dev->attached_devices + 1;
        u64 devblocks;

	const char **lines = NULL;
	char *line;
	char *msg;
	lines = kzalloc(lcount * sizeof(char *), GFP_KERNEL);

	line =
	    as_sprintf("%7s %20s %15s %15s %15s %15s %15s %15s\n", "TIER", "DEVICE",
		       "SIZE MB", "ALLOCATED MB", "AVERAGE READS",
		       "AVERAGE WRITES", "TOTAL_READS", "TOTAL_WRITES");
	lines[0] = line;
	for (count = 0; count < dev->attached_devices; count++) {
		allocated = allocated_on_device(dev, count);
		allocated >>= BLKBITS;
                devblocks=dev->backdev[count]->devicesize >> BLKBITS;
                dev->backdev[count]->devmagic->average_reads =
                             dev->backdev[count]->devmagic->total_reads / devblocks;
                dev->backdev[count]->devmagic->average_writes =
                             dev->backdev[count]->devmagic->total_writes / devblocks;
		line =
		    as_sprintf("%7u %20s %15llu %15llu %15u %15u %15llu %15llu\n", count,
			       dev->backdev[count]->tier_dta_file->
			       f_dentry->d_name.name,
			       dev->backdev[count]->devicesize >> BLKBITS,
			       allocated,
			       dev->backdev[count]->devmagic->average_reads,
			       dev->backdev[count]->devmagic->average_writes,
                               dev->backdev[count]->devmagic->total_reads,
                               dev->backdev[count]->devmagic->total_writes);
		lines[count + 1] = line;
	}
	msg = as_strarrcat(lines, count + 1);
	while (count) {
		kfree((char *)lines[--count]);
	}
	kfree(lines);
	res = snprintf(buf, 1023, "%s\n", msg);
	kfree(msg);
	return (res);
}

TIER_ATTR_RW(sequential_landing);
TIER_ATTR_RW(migrate_verbose);
TIER_ATTR_RW(ptsync);
TIER_ATTR_RW(barriers);
TIER_ATTR_RW(migration_interval);
TIER_ATTR_RW(migration_enable);
TIER_ATTR_RW(migration_policy);
TIER_ATTR_RW(resize);
TIER_ATTR_RO(attacheddevices);
TIER_ATTR_RO(numreads);
TIER_ATTR_RO(numwrites);
TIER_ATTR_RO(device_usage);

static struct attribute *tier_attrs[] = {
	&tier_attr_sequential_landing.attr,
	&tier_attr_migrate_verbose.attr,
	&tier_attr_ptsync.attr,
	&tier_attr_barriers.attr,
	&tier_attr_migration_interval.attr,
	&tier_attr_migration_enable.attr,
	&tier_attr_migration_policy.attr,
	&tier_attr_attacheddevices.attr,
	&tier_attr_numreads.attr,
	&tier_attr_numwrites.attr,
	&tier_attr_device_usage.attr,
	&tier_attr_resize.attr,
	NULL,
};
