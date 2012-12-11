<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:cdis="http://www.freedesktop.org/Standards/CDIS">
<xsl:output method="html" />
<xsl:template match="/">
<html>
<body>
        <h1>CDIS Interface <xsl:value-of select="/cdis:DesktopService/@cdis:name" /></h1>
        <h2><xsl:value-of select="/cdis:DesktopService/cdis:Interface" />: <xsl:value-of select="/cdis:DesktopService/cdis:Description" /></h2>

        <h3>Exposed Events</h3>
        <ul>
                <xsl:for-each select="/cdis:DesktopService/cdis:ExposedEvents/cdis:Event">
                <li><h4><xsl:value-of select="@cdis:name" /></h4>
                        <h5><xsl:value-of select="cdis:Description" /></h5>
                        <xsl:if test="count(cdis:Arguments/cdis:Argument) &gt; 0">
                                <xsl:apply-templates select="cdis:Arguments" />
                        </xsl:if>
                </li>
                </xsl:for-each>
        </ul>
        
        <h3>Exposed Methods</h3>
        <ul>
                <xsl:for-each select="/cdis:DesktopService/cdis:ExposedMethods/cdis:Method">
                <li><h4><xsl:value-of select="@cdis:name" /></h4>
                        <h5><xsl:value-of select="cdis:Description" /></h5>
                        <xsl:if test="count(cdis:Arguments/cdis:Argument) &gt; 0">
                                <xsl:apply-templates  select="Arguments" />
                        </xsl:if>
                        <xsl:if test="count(cdis:ReturnValues/cdis:ReturnValue) &gt; 0">
                                <xsl:apply-templates select="cdis:ReturnValues" />
                        </xsl:if>
                </li>
                </xsl:for-each>
        </ul>
</body>
</html>
</xsl:template>

<xsl:template match="cdis:Arguments|cdis:ReturnValues">
        <h6><xsl:value-of select="name()" /></h6>
        <table border="1" width="100%">
                <tr>
                        <th>Name</th>
                        <th>Type</th>
                        <th>Description</th>
                </tr>
                <xsl:apply-templates>
                        <xsl:sort select="@cdis:id" data-type="number" />
                </xsl:apply-templates>
        </table>
</xsl:template>

<xsl:template match="cdis:Argument|cdis:ReturnValue">
        <tr>
                <td><xsl:value-of select="@cdis:name" /></td>
                <td><xsl:value-of select="cdis:Type" /></td>
                <td><xsl:value-of select="cdis:Description" /></td>
        </tr>
</xsl:template>
</xsl:stylesheet>
