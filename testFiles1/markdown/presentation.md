# UCLA CS130
# Team Just Enough
## Webserver Demo
### Dong Lee, Filisha Shah, Evan Krause

<style> @import url(mdstyle.css); </style>

## HTTP Authentication
 Our server implements HTTP Basic Authentication.
 You can configure what URIs require authentication, and allowed users.
 In our config:

    # by default, all paths are public
    path / StaticHandler {
        root testFiles1;
    }
    path /echo EchoHandler {}
    
    # password protected areas:
    path /secret/files StaticHandler {
        root testFiles2;
    }
    path /secret/status StatusHandler {}
    path /secret/echo EchoHandler {}
    
    # define secret "realm" with users
    realm /secret {
        credentials cs130 password;
        credentials user2 p@ssw0rd;
    }

Anyone can access [/cat.gif](/cat.gif), etc.

However, if you try to access [/secret/files/private.txt](/secret/files/private.txt),
you will have to enter a username and password.
You can only see the content if you enter "cs130" and "password" correctly.

Note that since realms are specified as a path separate to any handlers,
multiple handlers can apply to the same realm.

This feature allows server admins to easily block off parts of their site,
without requiring a full-blown cookie/session setup.
For example, the webserver status page [/secret/status](/secret/status) may show internal information they only want
developers to see.

Your browser will automatically remember passwords, usually until you restart.
All files in the same "realm" use the same authentication, so
if you authenticate on one page, your browser will automatically authenticate you
on the other pages.

If you go to [/secret/echo](/secret/echo) you can see the `Authorization` header in your request.

## Markdown Conversion

Markdown (.md) files are automatically converted into HTML when requested.
This page is written and served as markdown, translated on the fly.
You can even include css from inside.

Also in our configuration:

    path /boring StaticHandler {
       root files;
    }
    path /fancy StaticHandler {
       root files;
       convert_markdown true;
    }

Now [/boring/test.md](/boring/test.md) and [/fancy/test.md](/fancy/test.md)
point to the same file on disk,

but [/boring/test.md](/boring/test.md) does not convert the markdown,
and [/fancy/test.md](/fancy/test.md) does.

For example, go to [/boring/presentation.md](/boring/presentation.md) to see the non-translated markdown
of this page.

