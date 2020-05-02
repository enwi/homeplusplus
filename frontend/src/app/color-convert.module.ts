import { NgModule, Pipe, PipeTransform } from '@angular/core';

export function colorToHex(color: number): string {
  return '#' + color.toString(16).padStart(6, '0');
}

export function hexToColor(hex: string): number {
  return Number.parseInt(hex.substr(1), 16);
}

export function hueSatToHex(hue: number, saturation: number): string {
  // Normalize saturation between 0 and 1
  const s = Math.max(0, Math.min(saturation / 254, 1.0));
  const v = 1.0;

  const c = v * s;
  const x = c * (1 - Math.abs((hue / 60) % 2 - 1));
  const m = v - c;

  let r1, g1, b1;
  if (hue < 60) {
    r1 = c;
    g1 = x;
    b1 = 0;
  } else if (hue < 120) {
    r1 = x;
    g1 = c;
    b1 = 0;
  } else if (hue < 180) {
    r1 = 0;
    g1 = c;
    b1 = x;
  } else if (hue < 240) {
    r1 = 0;
    g1 = x;
    b1 = c;
  } else if (hue < 300) {
    r1 = x;
    g1 = 0;
    b1 = c;
  } else {
    r1 = c;
    g1 = 0;
    b1 = x;
  }

  const r = Math.floor((r1 + m) * 255);
  const g = Math.floor((g1 + m) * 255);
  const b = Math.floor((b1 + m) * 255);
  return '#' + r.toString(16).padStart(2, '0')
    + g.toString(16).padStart(2, '0')
    + b.toString(16).padStart(2, '0');
}

export function hexToHueSat(hex: string): [number, number, number] {
  const r = Number.parseInt(hex.substr(1, 2), 16) / 255;
  const g = Number.parseInt(hex.substr(3, 2), 16) / 255;
  const b = Number.parseInt(hex.substr(5, 2), 16) / 255;

  const cmax = Math.max(r, g, b);
  const cmin = Math.min(r, g, b);
  const delta = cmax - cmin;

  let hue;
  if (delta === 0) {
    hue = 0;
  } else if (cmax === r) {
    hue = 60 * ((g - b) / delta % 6);
  } else if (cmax === g) {
    hue = 60 * ((b - r) / delta + 2);
  } else {
    // cmax === b
    hue = 60 * ((r - g) / delta + 4);
  }
  const saturation = cmax === 0 ? 0 : Math.floor(delta / cmax * 255);
  const value = cmax;
  return [hue, saturation, value];
}

@Pipe({ name: 'hexColor' })
export class HexColorConvertPipe implements PipeTransform {
  transform(value: number): string {
    return colorToHex(value);
  }
}

@NgModule({
  declarations: [HexColorConvertPipe],
  exports: [HexColorConvertPipe]
})
export class ColorConvertModule {
}
