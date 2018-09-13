tupper
======

tupper helps to build and manage container images primarily for use with
integration testing. Containers can be built in a layered manner, with multiple
images sharing common filesystems for efficiency. Under the hood,
`systemd-nspawn` and `overlayfs` are used to provision the container instances.

Installing
----------

### Via source

```
# git clone git@github.com/mattferris/tupper.git tupper
# cp -p tupper/usr/bin/tup /usr/bin/
# cp -rp tupper/usr/lib/tupper /usr/lib/
# cp -rp tupper/var/lib/tupper /var/lib/
# apt-get install systemd-container
```

### Via pre-built package

```
# wget -O- https://pkg.bueller.ca/debian/pkg.bueller.ca.gpg > /etc/apt/trusted.gpg.d/pkg.bueller.ca.gpg
# echo "deb http://pkg.bueller.ca/debian stretch main" > /etc/apt/sources.list.d/pkg.bueller.ca.list
# apt-get update && apt-get install tupper
```

In the beginning...
-------------------

The easiest way to get started is to use `debootstrap` to prepare the container's
initial filesystem.

```
# apt-get install debootstrap
# deboostrap stable /home/user/debian9
```

Before we can create a container based on this filesystem, we'll need to *commit*
it. This adds the filesystem to tupper's object store. When committed, tupper
takes a checksum of the filesystem which it can use to verify the filesystem's
integrity. An optional message can be provided during the commit.

```
# tup commit /home/user/debian9 "Debian 9 base install"
commited 6f89a622d606a69e06621d818598e584249f5154 (/home/user/debian9)
```

Now we can create an image based on the newly commited layer.

```
# tup create 6f89 foo
created /var/lib/tupper/images/foo from 6f89a622d606a69e06621d818598e584249f5154
```

Layers are identified by their checksums. For simplicity, only part of the
checksum is required. In the above example, tupper will expland `6f89` to the
actual layer ID `6f89a622d606a69e06621d818598e584249f5154`.

Finally, we can run the container.

```
# tup run foo
```

Tupper will boot the container image, eventually supplying you with a console
login prompt. There's no root password set by default, so it's not possible to
login at the moment. You can kill the container by pressing `^]` or (ctrl+])
three times.

To change the password, we can run the container in a different way. Instead of
booting the container, we can simply run a command in the container's
namespace. This is done by supplying the command when running the container.

```
# tup run foo /bin/bash
```

This will give you a bash shell within the container, and allow you to change
the root password via `passwd`. Once done, type `exit` or (ctrl+d) to exit the
shell and stop the container.

Perfect! You're first container is up and running. Now is probably a good time
to commit this image and use it as a base for the rest of your images.

It's all about commitment
-------------------------

Committing an image is like taking a snapshot of it's filesystem. Once an image
is committed, you can always return to the snapshot at a later date. You can
also build additional images using the snapshot as a base.

First, let's commit the changes in the `foo` image.

```
# tup commit foo 'set root password'
committed e95e7547cff2042631b52e9c19da74212fb8d28b (foo)
```

Once complete, the command will display the layer ID. This is a SHA1 checksum
that can be used to verify the contents of the commit. This ensures that you're
able to confirm that the contents of the commit is that same as when it was
originally committed.

Verifying a commit is easy.

```
# tup verify e95e
verifying e95e7547cff2042631b52e9c19da74212fb8d28b
layer e95e7547cff2042631b52e9c19da74212fb8d28b ok
layer 6f89a622d606a69e06621d818598e584249f5154 ok
verification successful
```

Layers are linked to their parents. In this case, `6f89` is the parent of
`e95e`. A layers checksum is based on the checksum of it's filesystem and it's
parents checksum. This means that a layer is only valid if all it's ancestors
are also valid. You can see a layers ancestry (or *lineage*) like so:

```
# tup lineage e95e
e95e7547cff2042631b52e9c19da74212fb8d28b <root@example> set root password
6f89a622d606a69e06621d818598e584249f5154 <root@example> Debian 9 base image
```

Let's clean up a bit and remove the `foo` image.

```
# tup rm foo
```

Now, how can all this be used to your advantage?

Layers, like an onion
---------------------

Say you have a project, call it *project-x*. You need a convenient way of
testing your progress that won't break your existing system. Containers provide
a lightweight, flexible solution. You've already prepared a basic image with a
root password, why not extend that image further to suit your needs?

Create a new image based off of the latest commit made from the `foo` image.

```
# tup create foo project-x
created /var/lib/tupper/images/project-x from e95e7547cff2042631b52e9c19da74212fb8d28b
```

Now you have an new image based on the latest commit of the `foo` image. Even
though we removed the `foo` image, tupper still knows the last commit the image
as pointing to. This is because when a commit is made on an image, tupper
creates a *tag* pointing at the layer ID. You can view tags and their related
layer IDs like so:

```
# tup tag
foo -> e95e7547cff2042631b52e9c19da74212fb8d28b
```

*Project-x* has a few dependecies that need to be installed before we can start
using the container for testing. Before we can do that though, we need to setup
networking. In particular, we need to specify a bridge interface to connect the
container to. Let's assume `br0` is available, and bridges onto to DHCP network.

```
# tup set project-x net br0
```

Launch a shell in the container to configure the interface to use DHCP.

```
# tup run project-x /bin/bash
# cat <<EOF > /etc/networking/interfaces.d/host0
allow-hotplug host0
iface host0 inet dhcp
EOF
```

Without exiting the shell, let's bring up the interface and install the
dependencies as well.

```
# ifup host0
# apt-get install rsync less cowsay
```

Finally, exit the shell.

Nice, everything is ready for testing now. Before that though, let's make sure
we commit the image to ensure we can undo the changes from our test runs.

```
# tup commit project-x 'setup, and ready to go'
committed 00bb86a10fcec0d1fd9fd1388fefe7fef3bb0ee6 (project-x)
```

Finally, let's see how we this all comes together to enable re-producible test
environments.

Reduce, re-use, recycle
-----------------------

[coming soon]
