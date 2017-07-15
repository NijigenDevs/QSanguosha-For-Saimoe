#include "util.h"
#include "sol.hpp"

#include <QVariant>
#include <QStringList>
#include <QMessageBox>

extern "C" {
    int luaopen_sgs(lua_State *);
}

QVariant GetValueFromLuaState(sol::state &L, const char *table_name, const char *key)
{

	auto c = L[table_name][key];
	if (!c.valid()) {
		QMessageBox::warning(NULL, "Lua warning", "invalided object at:" + QString(table_name) +"[" + key + "]");
		return QVariant();
	} else {
		QVariant data;
		switch (c.get_type()) {
		case sol::type::string: {
			std::string temp = c.get_or<std::string>("");
			data = QString::fromUtf8(temp.data());
			break;
		}
		case sol::type::number: {
			data = c.get_or<double>(0);
			break;
		}
		case sol::type::table: {

			sol::table t = c;

			QStringList sl;
			QVariantMap vm;
			bool array = true;

			t.for_each([&sl,&array,&vm](sol::object &index, sol::object &value) {
				if (!index.is<int>())
					array = false;
				if (array) { 
					sl.append(QString::fromUtf8(value.as<std::string>().data()));
				} else {
					if (index.is<std::string>()) {
						vm[QString::fromUtf8(index.as<std::string>().data())] = QString::fromUtf8(value.as<std::string>().data());
					}
				}

			});
			if (array) data = sl; else data = vm;
			break;
		}
		case sol::type::nil: {
			QMessageBox::warning(NULL, "Lua warning", "object at:" + QString(table_name) + "[" + key + "]" + " is nil!");
			break;
		}
		default:
			break;
		}

		return data;
	}
}

// lua_State *CreateLuaState()
// {
//     lua_State *L = luaL_newstate();
//     luaL_openlibs(L);
//     luaopen_sgs(L);
// 
//     return L;
// }
// 
void DoLuaScript(sol::state &L, const char *script)
{
    auto result = L.do_file(script);
	if (!result.valid()) {
		sol::error e = result;
		QMessageBox::critical(NULL, QObject::tr("Lua script error"), e.what());
		exit(1);
	}
//     if (error) {
//         QString error_msg = lua_tostring(L, -1);
//         QMessageBox::critical(NULL, QObject::tr("Lua script error"), error_msg);
//         exit(1);
//     }
}

QStringList IntList2StringList(const QList<int> &intlist)
{
    QStringList stringlist;
    for (int i = 0; i < intlist.size(); i++)
        stringlist.append(QString::number(intlist.at(i)));
    return stringlist;
}

QList<int> StringList2IntList(const QStringList &stringlist)
{
    QList<int> intlist;
    for (int i = 0; i < stringlist.size(); i++) {
        QString n = stringlist.at(i);
        bool ok;
        intlist.append(n.toInt(&ok));
        if (!ok) return QList<int>();
    }
    return intlist;
}

QVariantList IntList2VariantList(const QList<int> &intlist)
{
    QVariantList variantlist;
    for (int i = 0; i < intlist.size(); i++)
        variantlist.append(QVariant(intlist.at(i)));
    return variantlist;
}

QList<int> VariantList2IntList(const QVariantList &variantlist)
{
    QList<int> intlist;
    for (int i = 0; i < variantlist.size(); i++) {
        QVariant n = variantlist.at(i);
        bool ok;
        intlist.append(n.toInt(&ok));
        if (!ok) return QList<int>();
    }
    return intlist;
}

bool isNormalGameMode(const QString &mode)
{
    return mode.endsWith("p") || mode.endsWith("pd") || mode.endsWith("pz");
}
