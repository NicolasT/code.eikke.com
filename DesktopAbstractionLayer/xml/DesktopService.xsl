<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:dis="http://www.freedesktop.org/Standards/DIS">
<xsl:output method="html" />
<xsl:template match="/">
<html>
<body>
        <h1>DesktopService <xsl:value-of select="/dis:DesktopService/@name" /></h1>
        <h2><xsl:value-of select="/dis:DesktopService/dis:Interface" />: <xsl:value-of select="/dis:DesktopService/dis:Description" /></h2>

        <h3>Exposed Events</h3>
        <ul>
                <xsl:for-each select="/dis:DesktopService/dis:ExposedEvents/dis:Event">
                <li><h4><xsl:value-of select="@dis:name" /></h4>
                        <h5><xsl:value-of select="dis:Description" /></h5>
                        <xsl:if test="count(dis:Arguments/dis:Argument) &gt; 0">
                                <xsl:apply-templates select="dis:Arguments" />
                        </xsl:if>
                </li>
                </xsl:for-each>
        </ul>
        
        <h3>Exposed Methods</h3>
        <ul>
                <xsl:for-each select="/dis:DesktopService/dis:ExposedMethods/dis:Method">
                <li><h4><xsl:value-of select="@dis:name" /></h4>
                        <h5><xsl:value-of select="dis:Description" /></h5>
                        <xsl:if test="count(dis:Arguments/dis:Argument) &gt; 0">
                                <xsl:apply-templates  select="Arguments" />
                        </xsl:if>
                        <xsl:if test="count(dis:ReturnValues/dis:ReturnValue) &gt; 0">
                                <xsl:apply-templates select="dis:ReturnValues" />
                        </xsl:if>
                </li>
                </xsl:for-each>
        </ul>
</body>
</html>
</xsl:template>

<xsl:template match="dis:Arguments|dis:ReturnValues">
        <h6><xsl:value-of select="name()" /></h6>
        <table border="1" width="100%">
                <tr>
                        <th>Name</th>
                        <th>Type</th>
                        <th>Description</th>
                </tr>
                <xsl:apply-templates>
                        <xsl:sort select="@dis:id" data-type="number" />
                </xsl:apply-templates>
        </table>
</xsl:template>

<xsl:template match="dis:Argument|dis:ReturnValue">
        <tr>
                <td><xsl:value-of select="@dis:name" /></td>
                <td><xsl:value-of select="dis:Type" /></td>
                <td><xsl:value-of select="dis:Description" /></td>
        </tr>
</xsl:template>
</xsl:stylesheet>
