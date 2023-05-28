#include <QtCore/QCoreApplication>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/qmetaobject.h>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusVariant>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <assert.h>

extern "C" {
#include "querydbus.h"
}
 
QT_BEGIN_NAMESPACE
Q_DBUS_EXPORT extern bool qt_dbus_metaobject_skip_annotations;
QT_END_NAMESPACE
 
static QDBusConnection connection(QLatin1String(""));

void qd_create()
{
  connection = QDBusConnection::sessionBus();
  assert(connection.isConnected());
}
 
static int qd_query_dbus(const char *service_str, const char *path_str,
  const char *member_str, const char *argument_str)
{
  QT_PREPEND_NAMESPACE(qt_dbus_metaobject_skip_annotations) = true;

  assert(connection.isConnected());
 
  QString service(service_str);
  QString path(path_str);
  QString member(member_str);
  QString argument(argument_str);

  QString interface;
  QDBusInterface iface(service, path, interface, connection);

  const QMetaObject *meta_object = iface.metaObject();
  QByteArray match = member.toLatin1();
  match += '(';

  int method_id = -1;
  for (int i = meta_object->methodOffset(); i < meta_object->methodCount(); ++i)
  {
    QMetaMethod meta_method = meta_object->method(i);
    QByteArray signature = meta_method.methodSignature();
    if (signature.startsWith(match))
    {
      method_id = i;
      break;
    }
  }
  assert(method_id >= 0);
  QMetaMethod meta_method = meta_object->method(method_id);

  QList<QByteArray> types = meta_method.parameterTypes();
  assert(types.count() == 1);
    
  int type_id = QVariant::nameToType(types.at(0));
  assert(type_id);
  assert(type_id != QVariant::UserType);
  assert(type_id != QVariant::List);
  assert(type_id != QVariant::StringList);
  assert(type_id != QVariant::Map);
  assert(type_id < QMetaType::User);

  QVariant param = argument;
  param.convert(type_id);
  assert(param.type() != QVariant::Invalid);

  QVariantList params;
  params += param;

  QDBusMessage reply = iface.callWithArgumentList(QDBus::Block, member, params);

  return reply.type() == QDBusMessage::ReplyMessage ? 0 : -1;
}

void qd_volume_up()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/mediacontrol", "invokeShortcut", "mediavolumeup");
  assert(ret >= 0);
}

void qd_volume_down()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/mediacontrol", "invokeShortcut", "mediavolumedown");
  assert(ret >= 0);
}

void qd_audio_prev()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/mediacontrol", "invokeShortcut", "previousmedia");
  assert(ret >= 0);
}

void qd_audio_next()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/mediacontrol", "invokeShortcut", "nextmedia");
  assert(ret >= 0);
}

void qd_audio_play()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/mediacontrol", "invokeShortcut", "playpausemedia");
  assert(ret >= 0);
}

void qd_screen_off()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/org_kde_powerdevil", "invokeShortcut", "Turn Off Screen");
  assert(ret >= 0);
}

void qd_desk_1()
{
  int ret = qd_query_dbus("org.kde.KWin", "/KWin", "setCurrentDesktop", "1");
  assert(ret >= 0);
}

void qd_desk_2()
{
  int ret = qd_query_dbus("org.kde.KWin", "/KWin", "setCurrentDesktop", "2");
  assert(ret >= 0);
}

void qd_desk_3()
{
  int ret = qd_query_dbus("org.kde.KWin", "/KWin", "setCurrentDesktop", "3");
  assert(ret >= 0);
}

void qd_window_close()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/kwin", "invokeShortcut", "Window Close");
  assert(ret >= 0);
}

void qd_window_maximize()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/kwin", "invokeShortcut", "Window Maximize");
  assert(ret >= 0);
}

void qd_window_max_horiz()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/kwin", "invokeShortcut", "Window Maximize Horizontal");
  assert(ret >= 0);
}

void qd_window_max_vert()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/kwin", "invokeShortcut", "Window Maximize Vertical");
  assert(ret >= 0);
}

void qd_window_pack_down()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/kwin", "invokeShortcut", "Window Pack Down");
  assert(ret >= 0);
}

void qd_window_pack_up()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/kwin", "invokeShortcut", "Window Pack Up");
  assert(ret >= 0);
}

void qd_window_pack_left()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/kwin", "invokeShortcut", "Window Pack Left");
  assert(ret >= 0);
}

void qd_window_pack_right()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/kwin", "invokeShortcut", "Window Pack Right");
  assert(ret >= 0);
}

void qd_center_mouse()
{
  int ret = qd_query_dbus("org.kde.kglobalaccel", "/component/kwin", "invokeShortcut", "MoveMouseToCenter");
  assert(ret >= 0);
}
