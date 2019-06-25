## tup container

Responsible for managing ./containers/

Sub commands:

*	commit <ref> <msg>
*	create <name> [ <image> ]
*	destroy <ref>
	label { add | remove } <label>
*	list
	rebase <ref> <image>
!	revert <ref>
!	run <ref> <cmd>
	set <ref> <opt> <val>
*	show <ref>
*	start <ref>
!	status <ref>
!	stop <ref>
	unset <ref> <opt>

## tup image 

Responsible for managing ./images/

Sub commands:

	build <file>
	export <ref> <file>
	import <file>
*	create <name> { --layer <ref> | --path <path> } [ --plain | --squashfs | --tgz ] [ --labels <label>,... ] [ --boot | --entrypoint <cmd> ]
*	destroy <ref>
*	lineage <ref>
*	list
	pull <repo> <ref>
*	show <ref>
*	verify [ --recursive ] <ref>

## tup layer

Responsible for managing ./layers/

Sub commands:

*	create <path> [ --labels <label>,... ] [ --plain | --squashfs | --tgz  } ] [ --squashfs-opts <opts> ] [ --tar-opts <opts> ]
*	lineage <ref> [ --date ] [ --help ] [ --long-id ]
*	list [ --label <label> ]
*	squash { <ref> | <ref1>..<refn> }
*	mount <ref> [ --help ]
*	show <ref>
*	verify <ref> [ --help ] [ --lineage ] [ --long-id ]

## tup tag

Responsible for managing ./tags/

Sub commands:

	add <tag> <ref>
	list
	rm <tag>
	show <tag>

## tup volumes

Resonsible for managing ./volumes.

Sub commands:

*	create { --empty | --layer | --plain | --squashfs | --tgz } <source> ...
*	commit <ref> [ --labels | --extra-labels ]
*	destroy <ref>
*	list
*	show <ref>
+	status <ref>
*	unify [ <ref> ... ] [ --help ] [ --read-only | --read-write ] [ --stdin ]

## Utility commands

	ci <repo> <image> <cmd>
	fetch <repo> <image>
	gc [ --dry-run ] [ --expiry <hours> ] [ --force ] [ --help ]
*	list <object> [ --label <label> ] [ --long ]
	receive <image>
*	refparse <ref> [ --container | --image | --layer | --volume ] [ --help ]
	send <image>
*	show <ref> { --type <type> }
*	sum [ <dir> ]

# error codes

	  1 bad syntax, missing arguments
	  2 invalid arguments
	  3 prerequisites failed
	  4 internal command failed
	 10 object not found
        100 volume already mounted (soft)
