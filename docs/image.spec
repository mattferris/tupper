## image format

An image lives in the `images` directory. The directory is named after the
image. The directory contains the following *meta* files:

- `labels`: each line contains a label for the container
- `networks`: each line contains network to add the container to
- `options`: each line contains a container option
- `volumes`: each line contains a mountpoint and volume for the container

image-name/
	labels
	networks
	options
	volumes

When an image is *committed*, the image directory is copied to a temporary
directory in `staging`. Two additional *meta* files are added:

- `author`: the name and email address of the user making the commit
- `timestamp`: the date and time of the commit

Together, the contents of the *meta* files are concatenated and hashed to create
the *object ID*. The image directory is then moved to the `objects` directory,
named after the *object ID*.

## volume format

A *volume* is a directory in the `volumes` directory. Volume directories are
named after the filesystem *object ID*. In the case of ephemeral volumes, or
writeable volumes, the *object ID* is a hash of the container ID and the
volume's mountpoint. When committed, a volume becomes a filesystem object in
a new layer.

## layer format

A *layer* is a directory in the `objects` directory, named after the layer's
SHA256 checksum. A layer directory contains the following *meta* files:

- `labels`: each line contains a label for the layer
- `object`: the object ID (SHA256 checksum) of the filesystem
- `size`: the size of the filesystem, in bytes
- `type`: the format of the layer filesystem (i.e. plain, squashfs, tar)

A layer's *commit ID* is determined in a simliar manner to an image. The layer's
*meta* files are concatenated and hashed.


