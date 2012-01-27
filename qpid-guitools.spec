Name:           qpid-guitools
Version:        1.0.0
Release:        1%{?dist}
Summary:        GUI utilities for Red Hat MRG qpid   

License:        ASL 2.0
URL:            http://eallen.fedorapeople.org/
Source0:        http://eallen.fedorapeople.org/qpid-guitools.tar.gz
Group:          System Environment/Libraries

BuildRequires:  qt4-devel
BuildRequires:  qpid-qmf-devel
BuildRequires:  qpid-cpp-client-devel
BuildRequires:  cmake >= 2.6.0

Requires:       qt >= 4.6 
Requires:       qpid-qmf
Requires:       qpid-cpp-client

%description
qpid-guitools - Graphical tools built with qt that display
information about qpid brokers, queues, messages, exchanges, etc.

%prep
%setup -q -c ${name} 


%build
%cmake qpid-guitools
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%_bindir
cp xview %{buildroot}%_bindir/qpid-gbroker
cp qpid-guitools/README.txt .

%files
%defattr(-,root,root,-)
%_bindir/qpid-gbroker
%doc README.txt

%changelog

* Fri Jan 13 2012 Ernie Allen <eallen@redhat.com> 1.0.0-1
- Initial version containing /usr/bin/xview

