tupper
======

tupper helps to build and manage container images primarily for use with
integration testing. Containers can be built in a layered manner, with multiple
images sharing common filesystems for efficiency. Under the hood, `unionfs` and
`systemd-nspawn` are used to provision the container instances.

Setup
-----

Clone the repo into `/root/containers`. This is currently a hardcoded path, and
therefore required.

```
# git clone git@github.com/mattferris/tupper.git /root/containers
```

For now, all commands must be run from `/root/containers`.

In the beginning...
-------------------

The easiest way to get started is to use `debootstrap` to prepare the container's
initial filesystem. We'll install the new system into a new container image.

```
# apt-get install debootstrap
# mkdir -p /root/containers/img/deb9-amd64/fs
# deboostrap stable /root/containers/img/deb9/fs
```

As you can see, the container's filesystem lives in the `fs` folder within the
image folder.

Make sure you can boot the image.

```
# cd /root/containers
# ctl/run deb9
```

`ctl/run` should attempt to boot the container image, eventually supplying you
with a console login prompt. You can kill the container by pressing `^]` or
(ctrl+]) three times.

By default, the root account is disabled, so we need to set it to a known
password. This is easily done by copying a password from `/etc/shadow` on a
known system and pasting it into `root`'s password field in `/root/containers/img/deb9/fs/etc/shadow`.

Now boot the system again to make sure you can login. Once logged in, just run
`halt` to stop the container.

Take a peek inside the container's image folder.

```
# ls img/deb9
fs
```

Currently, the folder just contains the filesystem folder.

Perfect! You're first container is up and running. Now is probably a good time
to commit this image and use it as a base for the rest of your images.

It's all about commitment
-------------------------

Committing an image is like taking a snapshot of it's filesystem. Once an image
is committed, you can always return to the snapshot at a later date. You can
also build additional images using the snapshot as a base.

First, let's commit the changes in the `deb9` image.

```
# ctl/commit deb9 'committing base image for debian 9 (stretch)'
committed e95e7547cff2042631b52e9c19da74212fb8d28b (deb9)
```

Once complete, the command will display the commit ID. This is a SHA1 checksum
that can be used to verify the contents of the commit. This ensures that you're
able to confirm that the contents of the commit is that same as when it was
originally committed.

Verifying a commit is easy.

```
# ctl/verify e95e7547cff2042631b52e9c19da74212fb8d28b
verifying e95e7547cff2042631b52e9c19da74212fb8d28b
layer e95e7547cff2042631b52e9c19da74212fb8d28b ok
verification successful
```

Notice that contents of the image folder has changed.

```
# ls img/deb9
obj
```

Instead of the filesystem folder `fs`, it contains an `obj` file.

```
# cat img/deb9/obj
e95e7547cff2042631b52e9c19da74212fb8d28b
```

When the image was committed, it's filesystem was moved to a commit object, and
the image was updated with a pointer to the commit. The commit object can be
found in `/root/containers/obj/e95e7547cff2042631b52e9c19da74212fb8d28b`. The
contents of the `/root/containers/obj` folder is very much a "look, don't touch"
type of environment, as it's contents is managed by various commands.

Now, how can all this be used to your advantage?

Layers, like an onion
---------------------

Say you have a project, call it *project-x*. You need a convenient way of
testing your progress that won't break your existing system. Containers provide
a lightweight, flexible solution. You've already built a basic debian image, so
why not extend that image further to suit your needs?

Create a new image based off of the `deb9` image we already created.

```
# ctl/checkout deb9 img/project-x
checked out e95e7547cff2042631b52e9c19da74212fb8d28b to project-x
```

Now you have an new image based on the latest commit of the `deb9` image. The
contents of the image is just an `obj` file with the commit ID.

```
# cat img/project-x/obj
e95e7547cff2042631b52e9c19da74212fb8d28b
```

*Project-x* has a few dependecies that need to be installed before we can start
using the container for testing. Before we can do that though, we need to setup
networking. Boot the new image.

```
# ctl/run project-x
```

Once logged in, create the file `/etc/networking/interfaces.d/host0` with the
following.

```
auto host0
iface host0 inet dhcp
```

This assumes you're network has DHCP available.

Reboot the container to load the changes.

```
# reboot
```

Login once again, and install the dependecies.

```
# apt-get install rsync less cowsay
```

Now halt the container to stop it.

```
# halt
```

Nice, everything is ready for testing now. Before that though, let's make sure
we commit the image to ensure we can undo the changes from our test runs.

```
# ctl/commit project-x 'setup, and ready to go'
committed 00bb86a10fcec0d1fd9fd1388fefe7fef3bb0ee6 (project-x)
```

Our image now contains a number of layers (two, for now). It's possible to view
the layers (or *lineage*) of the image.

```
# ctl/lineage project-x
project-x
00bb86a10fcec0d1fd9fd1388fefe7fef3bb0ee6 <root@example.com> setup, and ready to go
e95e7547cff2042631b52e9c19da74212fb8d28b <root@example.com> committing base image for debian 9 (stretch)
```

Finally, let's see how we this all comes together to enable re-producible test
environments.

Reduce, re-use, recycle
-----------------------

[coming soon]
