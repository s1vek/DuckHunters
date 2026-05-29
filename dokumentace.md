# Duck Hunters

Arkádová střílečka inspirovaná hrou Duck Hunt.
Ze stran obrazovky vylétávají kachny a prolétávají ze strany na stranu, hráč
ovládá zaměřovač a snaží se je sestřelit. Cílem je nasbírat co nejvíc bodů,
než přijde o všechny životy.

---

## Uživatelský manuál

### Ovládání

Hra se ovládá třemi otočnými knoby na desce:

| Ovladač | Otáčení | Stisknutí |
|---|---|---|
| **Červený knob**| pohyb zaměřovače po ose X / navigace v menu | potvrzení volby v menu |
| **Modrý knob**| pohyb zaměřovače po ose Y | pauza (během hry) |
| **Zelený knob**| – | výstřel / potvrzení |

### Průběh hry

- Hráč začíná se **3 životy**.
- Trefená kachna přičte **bod** a zeleně blikne RGB LED.
- **Výstřel mimo** stojí **život** a červeně blikne RGB LED.
- Kachna, která **proletí nezasáhnutá**, stojí **život**.
- Rychlost kachen a frekvence jejich vylétávání se s rostoucím skóre
  **lineárně zvyšují** — hra je postupně těžší.
- Po ztrátě všech životů hra končí, zobrazí se aktuální skóre i dosavadní
  rekord. Rekord se ukládá a přežije i vypnutí.

### Obrazovky

Loading → hlavní menu (START / CONTROLS / EXIT) → hra → game over → zpět do menu.
Pauza je vyvolána stiskem modrého knobu, ovládání se zobrazí volbou CONTROLS v menu.

### Periferie

- **LCD displej** — vykreslování celé hry.
- **3 enkodéry (knoby)** — veškeré ovládání.
- **RGB LED** — zelená při zásahu, červená při minutí.
- **Řádka LED** — zobrazuje zbývající životy.

---

## Kompilace, instalace a spuštění

### Požadavky

- Křížový překladač `arm-none-linux-gnueabihf-gcc`.
- Připojená deska dostupná po síti (SSH).

### Překlad

```sh
make
```

Vznikne spustitelný soubor `duck_hunters`. Pro čistý překlad:

```sh
make clean && make
```

### Nahrání a spuštění na desce

IP adresu desky předáš proměnnou `TARGET_IP`:

```sh
make run TARGET_IP=192.168.0.123
```

Cíl `run` přeloží aplikaci, zkopíruje ji do `/tmp/<user>` na desce (proměnná
`TARGET_DIR`) a rovnou ji přes SSH spustí. Aplikaci lze ukončit volbou EXIT
v hlavním menu.

### Reset rekordu

Rekord se ukládá do souboru `duck_hunters_score.txt` v adresáři, odkud hra
běží. Smazáním souboru se rekord vynuluje:

```sh
rm /tmp/<user>/duck_hunters_score.txt
```

---

## Architektura aplikace

Aplikace je postavená na **přepínači scén**. `main()` namapuje periferie,
inicializuje displej a vstupy a spustí hlavní smyčku. Každá scéna má svou
funkci `*_run()`, která běží ve vlastní smyčce, dokud uživatel nevyvolá
přechod, a vrací další scénu (`scene_t`). Hlavní smyčka podle návratové
hodnoty přepíná mezi menu, hrou a obrazovkou ovládání.

```
                 +----------------+
   main() ─────► |  run_main_loop |  (přepínač scén)
                 +-------+--------+
                         |
        +----------------+----------------+
        |                |                |
   menu_run()       game_run()       control_run()
        |                |
        |          ducks_*  (logika kachen)
        |          render_scene  (vykreslení)
        |          pause_run  (pauza)
        |          score_*  (rekord)
        |
   (vše kreslí přes display_*, čte vstup přes input_*)
```

Stav hry drží jediná struktura `game_t` (pozice zaměřovače, skóre, životy,
pole kachen, časovače). Předává se herní logice (`ducks_*`) i vykreslování
(`render_scene`), takže logika a kreslení jsou oddělené.

### Struktura projektu

| Soubor | Obsah |
|---|---|
| `duck_hunters.c` | Vstupní bod, mapování periferií, přepínač scén |
| `display.c/.h` | LCD framebuffer a kreslení (obdélníky, text, flush) |
| `input.c/.h` | Čtení knobů a detekce hran tlačítek |
| `menu.c/.h` | Hlavní menu |
| `game.c/.h` | Herní smyčka, stav, LED, game over; sdílené typy a konstanty |
| `ducks.c/.h` | Spawnování, pohyb a kolize kachen |
| `render.c/.h` | Vykreslení herní scény (obloha, kachny, zaměřovač, HUD) |
| `control.c/.h` | Obrazovka s přehledem ovládání |
| `pause.c/.h` | Pauza |
| `score.c/.h` | Trvalé uložení rekordu |
| `scenes.h` | Výčet scén `scene_t` |
| `mzapo_*`, `serialize_lock.*`, `font_*` | Soubory z výukové šablony |

---

## Programátorská dokumentace

Stručný přehled hlavních funkcí jednotlivých modulů. Veřejné rozhraní je vždy
v příslušné hlavičce, `static` funkce jsou interní.

**display** — `display_init` inicializuje LCD, `display_clear` vyplní celý
framebuffer barvou, `display_rect` kreslí oříznutý obdélník, `display_text`
a `display_text_width` kreslí a měří text, `display_flush` pošle framebuffer
na displej. Barvy jsou v RGB565 přes makro `RGB(r, g, b)`.

**input** — `input_init` načte výchozí stav, `input_read` navzorkuje knoby
(spočítá delty otáčení a hrany tlačítek), `input_*_pressed` vrací 1 ve snímku,
kdy bylo tlačítko nově stisknuto.

**game** — `game_run` je hlavní herní smyčka: čte vstup, posouvá zaměřovač,
řeší výstřely, spawn a pohyb kachen, ztrátu životů, vykreslení a stav LED;
po konci uloží rekord a zobrazí game over. Interní `leds_update` řídí RGB LED
a řádku životů, `move_crosshair` posouvá a omezuje zaměřovač.

**ducks** — `ducks_try_spawn` přidá kachnu, pokud to pravidla dovolí, a řídí
časovač; `ducks_step` posune všechny kachny a vrátí počet těch, co opustily
obrazovku; `ducks_try_shoot` testuje zásah zaměřovačem.

**render** — `render_scene` vykreslí jeden kompletní snímek hry.

**menu / control / pause** — funkce `*_run` obsluhují příslušnou obrazovku
a vracejí další scénu.

**score** — `score_load_best` načte rekord ze souboru (0, pokud neexistuje),
`score_save_best` uloží nový rekord, jen pokud překonal ten dosavadní.
