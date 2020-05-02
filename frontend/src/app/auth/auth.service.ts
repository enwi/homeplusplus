import { Injectable, OnInit } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { BehaviorSubject, Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class AuthService {

  logout() {
    localStorage.removeItem('id_token');
    localStorage.removeItem('expires_at');
  }

  isLoggedIn(): boolean {
    // const loggedIn = this.checkLoggedIn();
    // if (!loggedIn && !this.userService.isUserValid()) {
    // }
    return this.checkLoggedIn();
  }

  getIdToken(): string {
    return this.checkLoggedIn() ? localStorage.getItem('id_token') : undefined;
  }

  private checkLoggedIn(): boolean {
    return this.getExpirationTime() > Math.floor(Date.now() / 1000);
  }

  getExpirationTime(): number {
    return +localStorage.getItem('expires_at');
  }

  login(authResult): boolean {
    if (authResult && authResult.idToken) {
      localStorage.setItem('id_token', authResult.idToken);
      localStorage.setItem('expires_at', authResult.expiresIn + Math.floor(Date.now() / 1000));
      return true;
    }
    return false;
  }

  // setUser(userResult): void {
  //   if (this.isLoggedIn()) {
  //     if (userResult && userResult.user && userResult.userid && userResult.pic) {
  //       this.userService.setUser(userResult.userid, userResult.user, userResult.pic);
  //     }
  //   }
  // }
}
