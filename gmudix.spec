Summary: A nice GTK+2 MUD client.
Name: gmudix
Version: 1.0
Release: 1
Copyright: GPL
Group: Amusements/Games
Source: http://dw.nl.eu.org/gmudix/gmudix-1.0.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot

%description
gMUDix is a GTK+2 MUD client.

%prep
%setup -q -n gmudix-1.0

%build
./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS README NEWS COPYING TODO
%doc doc/gmudix.txt


/usr/bin/gmudix

%changelog
* Tue May 26 2002 Sean Middleditch <elanthis@awesomeplay.com> - Initial spec file
- Added the .spec.in file for gMUDix
