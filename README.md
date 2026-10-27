# SUCCO — Vocal Chain Plugin

**SUCCO** è un plugin VST3 / AU / Standalone per catena vocale modulare, scritto in C++20 con [JUCE 8](https://github.com/juce-framework/JUCE). È un'implementazione **originale** ispirata all'architettura classica di catena vocale (gate → EQ → comp → de-ess → drive → double → chorus → delay → reverb → limit), con estetica UI dark purple.

> ⚠️ Questo progetto è un'opera originale. Non riproduce il codice sorgente, le risorse grafiche o il marchio di alcun plugin commerciale esistente. Il nome "SUCCO", il logo, il layout della UI e tutte le implementazioni DSP sono originali.

---

## Indice
- [Moduli DSP](#moduli-dsp)
- [Requisiti](#requisiti)
- [Build locale](#build-locale)
- [Build via CI (GitHub Actions)](#build-via-ci-github-actions)
- [Installazione del plugin](#installazione-del-plugin)
- [Struttura del progetto](#struttura-del-progetto)
- [Preset](#preset)
- [Limitazioni note in v1](#limitazioni-note-in-v1)
- [Licenza](#licenza)

---

## Moduli DSP

La catena fissa di elaborazione è, in ordine:

| # | Modulo    | Parametri principali                                       |
|---|-----------|-------------------------------------------------------------|
| 1 | **Tune**  | Semitoni (-12..+12), Amount (0..100%) — *pitch shifter*    |
| 2 | **Gate**  | Thresh, Range, Attack, Hold, Release                        |
| 3 | **EQ**    | HPF, Low Shelf, 2 Bell (low-mid, high-mid), High Shelf, LPF |
| 4 | **Comp**  | Thresh, Ratio, Attack, Release, Knee, Makeup               |
| 5 | **De-Ess**| Freq crossover, Thresh, Amount                              |
| 6 | **Drive** | Drive dB, Shape (soft→hard), Tone, Output — 2x OS           |
| 7 | **Double**| Amount, Time (ms), Spread (stereo width)                   |
| 8 | **Chorus**| Rate, Depth, Voices (1..4), Mix                             |
| 9 | **Delay** | Time, Feedback, Ping-Pong, Mix                              |
| 10| **Reverb**| Size, Decay, Damp, Predelay, LowCut, Mix (Freeverb-style)   |
| 11| **Limit** | Ceiling, Release, True-Peak (4x OS)                         |

Più master section: **Input / Punch / Mix / Output** (mix dry/wet globale).

---

## Requisiti

### Windows
- Windows 10/11 x64
- CMake ≥ 3.22
- Visual Studio 2022 (con workload *Desktop development with C++*)
- Connessione internet (CMake scaricherà JUCE e VST3 SDK tramite `FetchContent`)

### macOS
- macOS 11 (Big Sur) o superiore
- CMake ≥ 3.22
- Xcode Command Line Tools (`xcode-select --install`)
- Connessione internet

### Linux (opzionale)
- Ubuntu 22.04+ o equivalente
- CMake ≥ 3.22 + Ninja
- Dipendenze dev: ALSA, JACK, sndfile, FreeType2, X11, webkit2gtk-4.1

---

## Build locale

### 1. Clonare il repository

```bash
git clone https://github.com/<tuo-utente>/SUCCO.git
cd SUCCO
```

### 2. Configurare (Windows, Visual Studio 2022)

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Il file `SUCCO.vst3` sarà in:
```
build\SUCCO_artifacts\VST3\SUCCO.vst3
```

### 3. Configurare (macOS, universal binary arm64 + x86_64)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release --parallel
```

L'artefatto VST3 sarà in:
```
build/SUCCO_artifacts/VST3/SUCCO.vst3
```
Il bundle AU (se abilitato, default sì) in:
```
build/SUCCO_artifacts/AU/SUCCO.component
```

### 4. Configurare (Linux)

```bash
sudo apt install cmake ninja-build libasound2-dev libjack-jackd-dev \
  libsndfile1-dev libfreetype6-dev libx11-dev libxinerama-dev libxext-dev \
  libxrandr-dev libxcursor-dev libxcomposite-dev libxrender-dev \
  libwebkit2gtk-4.1-dev libcurl4-openssl-dev pkg-config

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### 5. Copiare il plugin nelle cartelle DAW

**Windows** (VST3):
```
C:\Program Files\Common Files\VST3\SUCCO.vst3
```
FL Studio e Reaper lo rileveranno automaticamente al prossimo avvio.

**macOS** (VST3):
```
/Library/Audio/Plug-Ins/VST3/SUCCO.vst3
~/Library/Audio/Plug-Ins/VST3/SUCCO.vst3     (per utente)
```

**macOS** (AU, opzionale):
```
/Library/Audio/Plug-Ins/Components/SUCCO.component
```

**Linux** (VST3):
```
~/.vst3/SUCCO.vst3
/usr/lib/vst3/SUCCO.vst3
```

> 💡 Su macOS il flag `SUCCO_COPY_PLUGIN_AFTER_BUILD=ON` (in CMake) copia il plugin nelle cartelle utente al termine del build.

### 6. Opzioni CMake disponibili

| Opzione                                 | Default | Descrizione                                |
|-----------------------------------------|---------|--------------------------------------------|
| `SUCCO_BUILD_VST3`                       | ON      | Compila il formato VST3                    |
| `SUCCO_BUILD_AU`                         | ON (Mac)| Compila il formato Audio Unit (solo Mac)  |
| `SUCCO_BUILD_STANDALONE`                | ON      | Compila l'app standalone                   |
| `SUCCO_BUILD_LV2`                        | OFF     | Compila il formato LV2 (solo Linux)        |
| `SUCCO_COPY_PLUGIN_AFTER_BUILD`         | OFF     | Copia il plugin nella cartella utente      |
| `JUCE_GIT_TAG`                           | 8.0.4   | Tag di JUCE da fetchare                    |

---

## Build via CI (GitHub Actions)

Il workflow in `.github/workflows/build.yml` compila automaticamente SUCCO su **Windows**, **macOS** (universal binary) e **Linux** ad ogni push.

### Per ottenere i binari

1. Forka o clona questo repo su GitHub.
2. Vai nella scheda **Actions**.
3. Trova il workflow **"Build SUCCO VST3"** e attendi il completamento.
4. Scarica gli artefatti (`.zip`) dalla scheda **Artifacts** in fondo al run.

Per le **release** taggate (es. `v1.0.0`):

```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub creerà automaticamente una release con i tre zip allegati.

---

## Installazione del plugin

### FL Studio (Windows / macOS)
1. Copia `SUCCO.vst3` nella cartella VST3 di sistema (vedi sopra).
2. Apri FL Studio → `Options → File → Manage plugins` → `Find more plugins`.
3. Cerca "SUCCO" e abilitalo.

### Reaper (Windows / macOS / Linux)
1. Copia `SUCCO.vst3` nella cartella VST3.
2. Reaper lo rileverà automaticamente. Apri `FX` su una traccia e cerca "SUCCO".

---

## Struttura del progetto

```
SUCCO/
├── CMakeLists.txt                  # Configurazione CMake + JUCE via FetchContent
├── README.md
├── LICENSE
├── .github/workflows/build.yml     # CI Windows + macOS + Linux
├── Assets/                         # Risorse grafiche (SVG)
└── Source/
    ├── PluginProcessor.h/.cpp      # AudioProcessor + APVTS + state save/load
    ├── PluginEditor.h/.cpp         # Editor principale + routing UI
    ├── dsp/
    │   ├── ChainProcessor.h/.cpp   # Routing 11 moduli + mix dry/wet
    │   ├── Gate.h
    │   ├── EQ.h
    │   ├── Compressor.h
    │   ├── DeEsser.h
    │   ├── Drive.h
    │   ├── Doubler.h
    │   ├── Chorus.h
    │   ├── Delay.h
    │   ├── Reverb.h
    │   ├── Limiter.h
    │   ├── PitchShift.h            # Modulo "Tune" (v1: pitch shift base)
    │   └── Oversampler.h           # Helper 2x/4x OS
    ├── gui/
    │   ├── SuccoLookAndFeel.h/.cpp # Tema dark purple + disegno knob custom
    │   ├── KnobComponent.h/.cpp     # Slider rotativo + label + tooltip IT
    │   ├── ModuleTabBar.h/.cpp      # Barra tab in alto
    │   └── DisplayComponent.h/.cpp  # Display centrale + meter + logo
    ├── params/
    │   └── ParameterManager.h/.cpp  # Layout APVTS completo (~60 params)
    └── presets/
        └── PresetManager.h/.cpp    # Factory presets + save/load user
```

---

## Preset

**Factory presets inclusi:**
- `Init` — stato neutro
- `Pop Lead` — catena vocale pop pronta
- `Rap / Trap` — aggressivo, compressione forte
- `Dreamy Lead` — chorus + delay + reverb ampio
- `Podcast Clean` — pulizia per podcast
- `Lo-Fi Hazy` — saturazione + chorus + LPF

**Preset utente:**
- Salvabili dal tab **PRESETS** (pulsante "Save Preset").
- Memorizzati in `~/Documents/SUCCO/Presets/*.succo`.

---

## Limitazioni note in v1

- **Modulo Tune**: in v1 implementa un **pitch-shifter** (modulazione di delay crossfade). Il pitch-correction automatico su scala richiederebbe un engine dedicato (YIN pitch detection + PSOLA); è pianificato per v2.
- **Modulo Punch**: il knob è attualmente un parametro passivo (mappato a un blend parallelo non ancora implementato nel ChainProcessor).
- **Sidechain esterno**: non ancora esposto come sidechain input.
- **Oversampling globale**: solo Drive e Limiter hanno OS locale; la catena intera gira a sample rate nativo.
- **Licensing VST3**: il plugin usa lo SDK VST3 di Steinberg distribuito con JUCE. La redistribuzione dei binari VST3 è soggetta al [VST3 License Agreement di Steinberg](https://www.steinberg.net/developers/). Per uso personale o distribuzione opensource non commerciale non è richiesta licenza individuale.

---

## Licenza

Codice sorgente distribuito sotto licenza **GPL-3.0-or-later** (vedi `LICENSE`).

Le librerie di terze parti rimangono sotto le rispettive licenze:
- [JUCE](https://github.com/juce-framework/JUCE): licenza ISC (parte del codice) / GPL-3 (per plugin GPLv3).
- VST3 SDK: licenza Steinberg (vedi sopra).

---

## Crediti

SUCCO è stato sviluppato come plugin vocale modulare originale. I concetti DSP sono basati su algoritmi pubblici standard (RBJ biquad, Freeverb, soft-clip tanh, lookahead limiter). L'estetica UI è dark-purple con knob custom-rendered.
