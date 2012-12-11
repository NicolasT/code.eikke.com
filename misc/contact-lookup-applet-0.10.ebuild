# Copyright 1999-2004 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License v2
# $Header: /home/cvsroot/bmg_overlay/gnome-extra/gnome-cpufreq-applet/gnome-cpufreq-applet-0.1.1.ebuild,v 1.1 2004/04/06 15:53:16 schick Exp $

inherit gnome2 eutils

DESCRIPTION="Contact search applet for the GNOME2 panel"
HOMEPAGE="http://packages.debian.org/unstable/gnome/contact-lookup-applet.html"
SRC_URI="http://ftp.debian.org/debian/pool/main/c/contact-lookup-applet/contact-lookup-applet_${PV}.orig.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86"
IUSE=""

RDEPEND="
	>=mail-client/evolution-2.0
	>=gnome-extra/evolution-data-server-1.0
	>=libglade-2.0
	>=gnome-base/gnome-panel-2.2"

DEPEND="${RDEPEND}"

src_unpack() {
	unpack ${A}

    cd ${S}
	 epatch ${FILESDIR}/gaim-integration.patch
	 
	autoheader
	autoconf
	automake -a
}

DOCS="AUTHORS COPYING ChangeLog NEWS README TODO"
