Summary: Future Composer input plugin for XMMS.
Name: xmms-fc
Version: 0.5.2
Release: 1
Source:	xmms-fc-%{version}.tar.gz
License: GPL
Group: Applications/Multimedia
URL: http://sourceforge.net/projects/xmms-fc/
BuildRoot: %{_tmppath}/%{name}-buildroot
BuildPrereq: xmms-devel
Requires: xmms >= 1.0.0
Prefix: %{_prefix}

%description
This is an input plugin for XMMS which can play back Future Composer
music files from AMIGA. Song-length detection and seek are
implemented, too.

%prep
rm -rf %{buildroot}

%setup -q -n xmms-fc-%{version}

%build
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{prefix}
make

%install
mkdir -p %{buildroot}
make DESTDIR=%{buildroot} install

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc COPYING README src/FC.README
%{prefix}/lib/xmms/Input/libfc.so
%{prefix}/lib/xmms/Input/libfc.la
