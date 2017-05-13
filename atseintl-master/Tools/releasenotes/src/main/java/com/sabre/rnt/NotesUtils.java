package com.sabre.rnt;
import org.apache.commons.lang3.StringEscapeUtils;

import java.net.UnknownHostException;

public class NotesUtils {

    /* This is for Apache Velocity templates - this class is used as a Tool. */
    public static final java.lang.String DEFAULT_KEY = "notesUtils";

    /**
     * If a string starts with "release_" prefix, then removes this prefix.
     */
    public static String assureNoReleasePrefix(String s) {
        String unwantedPrefix = "release_";
        if (s.startsWith(unwantedPrefix))
            return s.substring(unwantedPrefix.length(), s.length());
        return s;
    }

	/**
	 * @param s
	 * @return s with all line endings translated to the UNIX ones.
	 */
    public static String normalizeLineEndings(String s) {
    	return s.replaceAll("\\r\\n", "\n")
    			.replaceAll("\\r", "\n");
    }
    
    /**
     * Examples:
     * "[aaa]" => "aaa",
     * "aaa" => "aaa",
     * "[aaa" => "aaa",
     * "aaa]" => "aaa",
     * "[[aaa]]" => "[aaa]"
     */
	public static String removeSquareBrackets(String s) {
		int len = s.length();
		if (len == 0) return s;
		int offset = s.charAt(0) == '[' ? 1 : 0;
		int subLen = s.charAt(len - 1) == ']' ? len - 1 : len;
		return s.substring(offset, subLen);
	}


    public static String getCurrentInstanceURL() {
        String localHostname;
        try {
            localHostname = java.net.InetAddress.getLocalHost().getHostName() + ".dev.sabre.com";
        } catch (UnknownHostException e) {
            try {
                localHostname = java.net.InetAddress.getLocalHost().getHostAddress();
            } catch (UnknownHostException e1) {
                localHostname = "127.0.0.1";
            }
        }
        return localHostname;
    }

    public static String safeHTML(Object html) {
        //return html.replace("<", "&lt;").replace(">", "&gt;");
        return StringEscapeUtils.escapeHtml4(html.toString()).replace("$", "&#36;");
    }

    final static String OPEN_TMP_REPLACEMENT = "!!_open_!!";
    final static String CLOSE_TMP_REPLACEMENT = "!!_close_!!";
    final static String QUOTE_TMP_REPLACEMENT = "!!_quote_!!";

    private static String enc(String code) {
        return code.replaceAll("<", OPEN_TMP_REPLACEMENT).replaceAll(">", CLOSE_TMP_REPLACEMENT)
                .replaceAll("\"", QUOTE_TMP_REPLACEMENT);
    }

    private static String dec(String code) {
        return code.replaceAll(OPEN_TMP_REPLACEMENT, "<").replaceAll(CLOSE_TMP_REPLACEMENT, ">")
                .replaceAll(QUOTE_TMP_REPLACEMENT, "\"");
    }

    public static String unfilterTinyMCE(String htmlCode) {
        /* This function tries to revert changes made filterTinyMCE.
         * Actually, they aren't fully reversible, but we just want TinyMCE to be able to understand
         * the filtered code again, so only part of changes is reverted.
         */

        /* Q: Why are functions enc and dec used?
         * A: To make xml/html code visible in TinyMCE (the *code* itself, in the raw form),
         *    we have to HTML-encode it twice (first encoding is done to convey data to TinyMCE, the
         *    second one is done in the template).
         *    enc() and dec() help to preserve the tags that are *not* part of the raw code and should
          *     interpreted by TinyMCE.
         */

        String result = htmlCode;
        result = result.replaceAll("\\n", enc("<br>\n"));
        result = result.replaceAll("(?s)<bgcolor=([^>]*?)>(.*?)</bgcolor>", enc("<span style=\"background-color: $1\">$2</span>"));
        result = result.replaceAll("(?s)<color=([^>]*?)>(.*?)</color>", enc("<span style=\"color: $1\">$2</span>"));
        result = result.replaceAll("(?s)<code>(.*?)</code>", enc("<pre>$1</pre>"));
        result = result.replaceAll("(?s)<a([^>]*)>(.*?)</a>", enc("<a$1>$2</a>"));
        result = result.replaceAll("(?s)<b>(.*?)</b>", enc("<b>$1</b>"));
        result = result.replaceAll("(?s)<i>(.*?)</i>", enc("<i>$1</i>"));
        result = result.replaceAll("(?s)<u>(.*?)</u>", enc("<u>$1</u>"));
        result = result.replaceAll("(?s)<br>", enc("<br>"));
        return dec(safeHTML(result));
    }

    public static String filterTinyMCE(String htmlCode) {
        String result = StringEscapeUtils.unescapeHtml4(htmlCode);

        // (?s) - in from of regex works the same as Pattern.DOTALL

        //result = result.replaceAll("<br[^>]*>", "<br>\n");
        result = result.replaceAll("<br[^>]*>", "\n");
        result = result.replaceAll("(?s)<a.*?href=\\\"(.*?)\".*?>(.*?)</a>", "<a href=$1>$2</a>");
        result = result.replaceAll("(?s)<strong\\s*>(.*?)</strong\\s*>", "<b>$1</b>");
        result = result.replaceAll("(?s)<em\\s*>(.*?)</em\\s*>", "<i>$1</i>");
        result = result.replaceAll("(?s)<em\\s*>(.*?)</em\\s*>", "<i>$1</i>");
        result = result.replaceAll("(?s)</p>\\s*<p>", "\n").replace("<p>", "").replace("</p>", "");
        result = result.replaceAll("(?s)(<span [^>]+text-decoration: underline[^>]+>)(.*?)(</span>)", "$1<u>$2</u>$3");

        result = result.replaceAll("(?s)(<span [^>]*background-color: ([^;]*);[^>]*>)(.*?)(</span>)", "$1<bgcolor=$2>$3</bgcolor>$4");
        // Erase background-color information, to prevent interfering with foreground color below
        result = result.replaceAll("(?s)(<span [^>]*)background-color: [^;]*(;[^>]*>.*?</span>)", "$1$2");

        result = result.replaceAll("(?s)(<span [^>]*color: ([^;]*);[^>]*>)(.*?)(</span>)", "$1<color=$2>$3</color>$4");

        // Remove all the <span> tags, all needed information has been extracted from their styles, they
        // are no longer needed.
        result = result.replaceAll("<span[^>]+>", "").replaceAll("</span[^>]*>", "");

        result = result.replaceAll("(?s)<pre>(.*)</pre>", "<code>$1</code>");

        // Leave only ASCII printable characters (plus 0a and 0d).
        // This is a compromise: input comes from lots of places, like different Sabre terminals,
        // different browsers running on different OSes, etc., resulting in lots of kinds of garbage.
        result = result.replaceAll("[^\\x0a\\x0d\\x20-\\x7e]", " ");
        result = result.replaceAll("<\\s*(strong|em|span|pre)[^>]*>", "");

        result = result.replaceAll("(?s)\\n\\s*\\n", "<br>\n");

        // Remove lines with whitespace only
        result = result.replaceAll("(?m)^\\s+$", "");

        return result;
    }

    public static String produceFormName(String vanillaName) {
        return vanillaName.replace(" ", "_").toLowerCase();
    }
}


