%define plugindir %(xmms-config --input-plugin-dir)

Summary: Future Composer input plugin for XMMS.
Name: xmms-fc
Version: 0.5.4
Release: 1
URL: http://xmms-fc.sourceforge.net/
License: GPL
Source:	http://download.sourceforge.net/xmms-fc/xmms-fc-0.5.4.tar.bz2
Group: Applications/Multimedia
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot-%(%{__id_u} -n)
BuildRequires: xmms-devel, gtk+-devel
Requires: xmms

%description
This is an input plugin for XMMS which can play back Future Composer
music files from AMIGA. Song-length detection and seek are
implemented, too.


%prep
%setup -q -n xmms-fc-%{version}


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc COPYING README src/FC.README
%{plugindir}/libfc.so
%exclude %{plugindir}/libfc.la
