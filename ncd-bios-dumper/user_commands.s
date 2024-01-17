;;;
;;; Copyright (c) 2020 Damien Ciabrini
;;; This file is part of ngdevkit-examples
;;;
;;; ngdevkit is free software: you can redistribute it and/or modify
;;; it under the terms of the GNU Lesser General Public License as
;;; published by the Free Software Foundation, either version 3 of the
;;; License, or (at your option) any later version.
;;;
;;; ngdevkit is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU Lesser General Public License for more details.
;;;
;;; You should have received a copy of the GNU Lesser General Public License
;;; along with ngdevkit.  If not, see <http://www.gnu.org/licenses/>.
;;;

;;; user-specific commands to instantiate the nullsound driver
;;; ----------------------------------------------------------
        .module user_commands
        .area   CODE (REL)

;;; commands dependencies
;;;
        .include "ym2610.def"
        .include "nullsound.def"
        .include "user_commands.def"
        .include "sfx_adpcma.s"

        ;; [cmd 04][module 0] play ADPCM-A sample [0000..02ff] on channel 1
        snd_command_request 04, 0, sfx_adpcm_a_play, 0
snd_command_04_action_config:
        .dw     0x0                     ; sample start addr >> 8
        .dw     0x7ff                   ; sample stop addr >> 8
        .db     1                       ; channel 3
        .db     0xdf                    ; l/r output + volume

        ;; [cmd 05][module 0] play ADPCM-A sample [0300..0dff] on channel 1
        snd_command_request 05, 0, sfx_adpcm_a_play, 0
snd_command_05_action_config:
        .dw     0x0                     ; sample start addr >> 8
        .dw     0x3f                    ; sample stop addr >> 8
        .db     1                       ; channel 3
        .db     0xdf                    ; l/r output + volume

        ;; [cmd 06][module 1] play ADPCM-A sample [0e00..29ff] on channel 2, when not in use
        snd_command_request 06, 0, sfx_adpcm_a_play, 0
snd_command_06_action_config:
        .dw     0x0                     ; sample start addr >> 8
        .dw     0x7f                    ; sample stop addr >> 8
        .db     1                       ; channel 2
        .db     0xdf                    ; l/r output + volume

        ;; [cmd 07][module 0] play ADPCM-A sample [2A00..2CFF] on channel 1
        snd_command_request 07, 0, sfx_adpcm_a_play, 0
snd_command_07_action_config:
        .dw     0x40                    ; sample start addr >> 8
        .dw     0x7f                    ; sample stop addr >> 8
        .db     2                       ; channel 3
        .db     0xdf                    ; l/r output + volume

        ;; [cmd 08][module 0] play ADPCM-A sample [2A00..2CFF] on channel 1
        snd_command_request 08, 0, sfx_adpcm_a_play, 0
snd_command_08_action_config:
        .dw     0x80                    ; sample start addr >> 8
        .dw     0xbf                    ; sample stop addr >> 8
        .db     2                       ; channel 3
        .db     0xdf                    ; l/r output + volume

        ;; [cmd 09][module 0] play ADPCM-A sample [2A00..2CFF] on channel 1
        snd_command_request 09, 0, sfx_adpcm_a_play, 0
snd_command_09_action_config:
        .dw     0x800                   ; sample start addr >> 8
        .dw     0x83f                   ; sample stop addr >> 8
        .db     1                       ; channel 3
        .db     0xdf                    ; l/r output + volume

        ;; [cmd 0A][module 0] play ADPCM-A sample [2A00..2CFF] on channel 1
        snd_command_request 0A, 0, sfx_adpcm_a_play, 0
snd_command_0A_action_config:
        .dw     0x800                   ; sample start addr >> 8
        .dw     0x83f                   ; sample stop addr >> 8
        .db     1                       ; channel 3
        .db     0xdf                    ; l/r output + volume
