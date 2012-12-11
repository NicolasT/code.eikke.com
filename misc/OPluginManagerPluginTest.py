class OPluginManagerPluginAuthorData:
        def __init__(self, name, email = "", uri = ""):
                self.name = name
                self.email = email
                self.uri = uri

        def GetName(self):
                return self.name

        def GetEmail(self):
                return self.email

        def GetUri(self):
                return self.uri

        def Dump(self, prefix = ""):
                print prefix + self.name
                print prefix + "--------"
                if self.email != "":
                        print prefix + "* Email: " + self.email
                else:
                        print prefix + "* Email not set"
                if self.uri != "":
                        print prefix + "* Uri: " + self.uri
                else:
                        print prefix + "* Uri not set"
                print ""
                

class OPluginManagerPlugin:
        def __init__(self, name, summary, authors, description = "", version = "", uri = "", init_func = 0, data_free_func = 0, configure_func = 0):
                print "Creating new OPluginManagerPythonPlugin instance: " + name
                self.name = name
                self.summary = summary
                self.authors = authors
                self.description = description
                self.version = version
                self.uri = uri
                self.init_funcp = init_func
                self.data_free_funcp = data_free_func
                self.configure_funcp = configure_func

        def GetName(self):
                return self.name

        def GetSummary(self):
                return self.summary

        def GetAuthors(self):
                return self.authors

        def GetDescription(self):
                return self.description

        def GetVersion(self):
                return self.version

        def GetUri(self):
                return self.uri

        def Dump(self):
                print "Dumping data for plugin " + self.name
                print "-----------------------"
                print "* Name: " + self.name
                print "* Summary: " + self.summary
                print "* Authors: "
                self.authors.Dump("      ")
                if self.description != "":
                        print "* Description: " + self.description
                if self.version != "":
                        print "* Version: " + self.version
                if self.uri != "":
                        print "* Uri: " + self.uri
                if self.init_funcp != 0:
                        print "* Plugin got an init function"
                if self.data_free_funcp != 0:
                        print "* Plugin got a data_free function"
                if self.configure_funcp != 0:
                        print "* Plugin got a configure function"

        def configure_func(self):
                if self.configure_funcp != 0:
                        self.configure_funcp()

        def init_func(self, init_data):
                if self.init_funcp != 0:
                        return self.init_funcp(init_data)

        def data_free_func(self, data):
                if self.data_free_funcp != 0:
                        self.data_free_funcp(data)

class PythonTest (OPluginManagerPlugin):
        def __init__(self):
                tmpauthor = OPluginManagerPluginAuthorData("Ikke", "eikke eikke com", "http://www.eikke.com")
                OPluginManagerPlugin.__init__(self, "PythonTest", "A little test plugin", tmpauthor, "Some longer description of this test plugin", "0.1", "http://www.eikke.com", self.Init, self.FreeData, self.Configure)

        def Configure(self):
                print "Configuring"

        def Init(self, init_data):
                print "Initializing with data \"" + init_data + "\""
                return "testplugindata"

        def FreeData(self, data):
                print "Freeing \"" + data + "\""

def main():
        test = PythonTest()
        test.Dump()
        print ""
        tmp = test.init_func("init data")
        print "Init function returned: " + tmp
        test.configure_func()
        test.data_free_func(tmp)

if __name__ == "__main__":
        main()
