## Translations

### To add a new translation

1. Update your local repo to make sure everything is up to date.
2. Open `src/translations.pri` with your favorite text editor.
3. Add a new line that says `translations/baka-mplayer_{lang-code}.ts` in the format that exists (all lines except the last require a `\`).
4. Run `./configure CONFIG+=begin_translations` to prepare your translation file.
5. Go to `src/translations/` and you will now find your language's `.ts` file.
6. Open the `.ts` file with Qt Linguist and proceed to translate into your language.
7. Upon completion of the translation, run `./configure CONFIG+=update_translations`. This will trim the .ts file to the minimum required information for release.

### To update an existing translation

1. Update your local repo to make sure everything is up to date.
2. Run `./configure CONFIG+=begin_translations` to regenerate the `.ts` files.
3. Make your changes.
4. Run `./configure CONFIG+=update_translations` to trim the `.ts` files.

If you want to submit a translation, you can create a git pull request or send it to us through irc.

For more information on Qt Linguist (the program used to translate Qt projects) see http://qt-project.org/doc/qt-5/qtlinguist-index.html